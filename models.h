#include "cv.h"
#include "stats.h"
#include "utils.h"

/** 
 * Le modele Gaussien est represente par deux images a 3 channels. L'une des 
 * images contient la valeur moyenne, et l'autre, l'ecart-type.
 */
typedef struct _modeleGaussien
{
    IplImage* mean;
    IplImage* stdDev;
} GaussianModel;

/** 
 * Le modele median est represente par une image a 3 channels. Chacune contient
 * la valeur medianne a chaque pixel pour chaque couleur.
 */
typedef struct _modeleMedian
{
    float percentile;

    IplImage* median;
    IplImage* high;
    IplImage* low;
} MedianModel;

void initMedianModel(MedianModel* model, CvSize size, float percentile)
{
    model->percentile = percentile;

    model->median = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->median);

    model->high = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->high);

    model->low = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->low);
}

void initGaussianModel(GaussianModel* model, CvSize size)
{
    model->mean = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->mean);

    model->stdDev = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->stdDev);
}

void releaseMedianModel(MedianModel* model)
{
    if(model->median != NULL)
        cvReleaseImage(&model->median);
    if(model->high != NULL)
        cvReleaseImage(&model->high);
    if(model->low != NULL)
        cvReleaseImage(&model->low);
}

void releaseGaussianModel(GaussianModel* model)
{
    if(model->mean != NULL)
        cvReleaseImage(&model->mean);
    if(model->stdDev != NULL)
        cvReleaseImage(&model->stdDev);
}

void learnMedianModel(MedianModel* model, IplImage* frameBuffer[], int frameCount, float percentile)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);

    initMedianModel(model, fSize, percentile);
    IplImage *f; 
    IplImage *median = model->median, *pHigh = model->high, *pLow = model->low;

    char pixelsBlue[frameCount], pixelsGreen[frameCount], pixelsRed[frameCount];
    float medianBlue, medianGreen, medianRed;
    float pctHighBlue, pctHighGreen, pctHighRed;
    float pctLowBlue, pctLowGreen, pctLowRed;
    int step = median->widthStep, iStep;
    int row, col, i;
    for(row = 0; row < fSize.height; row++)
    {
        for(col = 0; col < fSize.width; col++)
        {
            for(i = 0; i < frameCount; i++)
            {
                f = frameBuffer[i];
                iStep = f->widthStep;

                pixelsBlue[i] = ((uchar*)(f->imageData + iStep*row))[col*3];
                pixelsGreen[i] = ((uchar*)(f->imageData + iStep*row))[col*3+1];
                pixelsRed[i] = ((uchar*)(f->imageData + iStep*row))[col*3+2];
            }

            // Calcul des mediannes
            medianBlue = computeMedian(pixelsBlue, frameCount);
            medianGreen = computeMedian(pixelsGreen, frameCount);
            medianRed = computeMedian(pixelsRed, frameCount);

            // Calcul des centiles
            pctHighBlue = computePercentile(pixelsBlue, frameCount, 
                model->percentile);
            pctHighGreen = computePercentile(pixelsGreen, frameCount, 
                model->percentile);
            pctHighRed = computePercentile(pixelsRed, frameCount, 
                model->percentile);

            pctLowBlue = computePercentile(pixelsBlue, frameCount, 
                (1.0 - model->percentile));
            pctLowGreen = computePercentile(pixelsGreen, frameCount, 
                (1.0 - model->percentile));
            pctLowRed = computePercentile(pixelsRed, frameCount, 
                (1.0 - model->percentile));

            // Positionne les valeurs mediannes dans le modele
            ((float*)(median->imageData + step*row))[col*3] = medianBlue; 
            ((float*)(median->imageData + step*row))[col*3+1] = medianGreen; 
            ((float*)(median->imageData + step*row))[col*3+2] = medianRed; 

            // Pourcentiles
            ((float*)(pHigh->imageData + step*row))[col*3] = pctHighBlue; 
            ((float*)(pHigh->imageData + step*row))[col*3+1] = pctHighGreen; 
            ((float*)(pHigh->imageData + step*row))[col*3+2] = pctHighRed; 

            ((float*)(pLow->imageData + step*row))[col*3] = pctLowBlue; 
            ((float*)(pLow->imageData + step*row))[col*3+1] = pctLowGreen; 
            ((float*)(pLow->imageData + step*row))[col*3+2] = pctLowRed; 
        }
    }
}

void learnGaussianModel(GaussianModel* model, IplImage* frameBuffer[], int frameCount)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);

    initGaussianModel(model, fSize);
    IplImage* f, *mean = model->mean, *sd = model->stdDev;

    uchar pixelsBlue[frameCount], pixelsGreen[frameCount], 
          pixelsRed[frameCount];
    float meanBlue, meanGreen, meanRed;
    float sdvBlue, sdvGreen, sdvRed;
    int stepm = mean->widthStep, steps = sd->widthStep, iStep;
    int row, col, i;
    for(row = 0; row < fSize.height; row++)
    {
        for(col = 0; col < fSize.width; col++)
        {
            for(i = 0; i < frameCount; i++)
            {
                f = frameBuffer[i];
                iStep = f->widthStep;

                pixelsBlue[i] = ((uchar*)(f->imageData + iStep*row))[col*3];
                pixelsGreen[i] = ((uchar*)(f->imageData + iStep*row))[col*3+1];
                pixelsRed[i] = ((uchar*)(f->imageData + iStep*row))[col*3+2];
            }
            
            // Calcul des parametres gaussiens
            computeMeanSdv(pixelsBlue, frameCount, &meanBlue, &sdvBlue);
            computeMeanSdv(pixelsGreen, frameCount, &meanGreen, &sdvGreen);
            computeMeanSdv(pixelsRed, frameCount, &meanRed, &sdvRed);

            // Positionne les valeurs de moyenne dans le modele
            ((float*)(mean->imageData + stepm*row))[col*3] = meanBlue; 
            ((float*)(mean->imageData + stepm*row))[col*3+1] = meanGreen; 
            ((float*)(mean->imageData + stepm*row))[col*3+2] = meanRed; 

            // Positionne les valeurs d'ecart-type dans le modele
            ((float*)(sd->imageData + steps*row))[col*3] = sdvBlue; 
            ((float*)(sd->imageData + steps*row))[col*3+1] = sdvGreen; 
            ((float*)(sd->imageData + steps*row))[col*3+2] = sdvRed; 
        }
    }
}

void learnAdaptiveGaussian(GaussianModel* model, char* dir, float alpha, int imageCount, int interval)
{
    CvSize fSize = cvSize(0, 0);

    IplImage* f = NULL, *mean = NULL;
    int stepm;

    char filename[256];
    char blue, green, red;
    float meanBlue, meanGreen, meanRed;

    int i;
    for(i = 0; i < imageCount; i += interval)
    {
        sprintf(filename, "%s/frame_%04d.jpg", dir, i);

        f = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(f == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        // Initialisation du modele Gaussien avec la premiere frame
        if(fSize.width == 0 && fSize.height == 0)
        {
            fSize = cvGetSize(f);
            initGaussianModel(model, fSize);
            cvCvtScale(f, model->mean, 1.0, 0);
            mean = model->mean;
            stepm = mean->widthStep;
        }

        // Mise a jour du modele Gaussien pixel par pixel
        int row, col, iStep;
        for(row = 0; row < fSize.height; row++)
        {
            for(col = 0; col < fSize.width; col++)
            {
                iStep = f->widthStep;

                // Valeurs courantes de moyennes
                meanBlue = ((float*)(mean->imageData + stepm*row))[col*3];
                meanGreen = ((float*)(mean->imageData + stepm*row))[col*3+1];
                meanRed = ((float*)(mean->imageData + stepm*row))[col*3+2];

                // Nouvelles valeurs de pixels
                blue = ((uchar*)(f->imageData + iStep*row))[col*3];
                green = ((uchar*)(f->imageData + iStep*row))[col*3+1];
                red = ((uchar*)(f->imageData + iStep*row))[col*3+2];
                
                // Mise a jour des valeurs moyennes
                meanBlue = alpha * blue + (1 - alpha) * meanBlue;
                meanGreen = alpha * green + (1 - alpha) * meanGreen;
                meanRed = alpha * red + (1 - alpha) * meanRed;
            
                // Positionne les nouvelles valeurs de moyenne dans le modele
                ((float*)(mean->imageData + stepm*row))[col*3] = meanBlue;
                ((float*)(mean->imageData + stepm*row))[col*3+1] = meanGreen;
                ((float*)(mean->imageData + stepm*row))[col*3+2] = meanRed;
            }
        }
    }
}
