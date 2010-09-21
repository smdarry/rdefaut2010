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
    IplImage* median;
} MedianModel;

void initMedianModel(MedianModel* model, CvSize size)
{
    model->median = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->median);
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
}

void releaseGaussianModel(GaussianModel* model)
{
    if(model->mean != NULL)
        cvReleaseImage(&model->mean);
    if(model->stdDev != NULL)
        cvReleaseImage(&model->stdDev);
}

void printArray(uchar a[], int count)
{
    FILE* fp = fopen("array.csv", "w+");
    int i;
    for(i = 0; i < count-1; i++)
    {
        fprintf(fp, "%d,", a[i]);
    }
    fprintf(fp, "%d", a[count-1]);

    fclose(fp);
}

void learnMedianModel(MedianModel* model, IplImage* frameBuffer[], int frameCount)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);

    initMedianModel(model, fSize);
    IplImage *f, *median = model->median;

    char pixelsBlue[frameCount], pixelsGreen[frameCount], pixelsRed[frameCount];
    float medianBlue, medianGreen, medianRed;
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
            medianBlue = 10; //computeMedian(pixelsBlue, frameCount);
            medianGreen = 20; //computeMedian(pixelsGreen, frameCount);
            medianRed = 30; //computeMedian(pixelsRed, frameCount);

            // Positionne les valeurs mediannes dans le modele
            ((float*)(median->imageData + step*row))[col*3] = medianBlue; 
            ((float*)(median->imageData + step*row))[col*3 + 1] = medianGreen; 
            ((float*)(median->imageData + step*row))[col*3 + 2] = medianRed; 
        }
    }
    printArray(pixelsBlue, frameCount);
    printFrame(median, 1, "medianImage.csv");
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
            ((float*)(mean->imageData + stepm*row))[col*3 + 1] = meanGreen; 
            ((float*)(mean->imageData + stepm*row))[col*3 + 2] = meanRed; 

            // Positionne les valeurs d'ecart-type dans le modele
            ((float*)(sd->imageData + steps*row))[col*3] = sdvBlue; 
            ((float*)(sd->imageData + steps*row))[col*3 + 1] = sdvGreen; 
            ((float*)(sd->imageData + steps*row))[col*3 + 2] = sdvRed; 
        }
    }
}
