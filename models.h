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
    IplImage* stdDev;
} MedianModel;

typedef struct _modelHist
{
    int height;
    int width;

    Histogram** hist;
} HistogramModel;

void initMedianModel(MedianModel* model, CvSize size, float percentile)
{
    model->percentile = percentile;

    model->median = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->median);

    model->high = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->high);

    model->low = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->low);

    model->stdDev = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->stdDev);
}

void initGaussianModel(GaussianModel* model, CvSize size)
{
    model->mean = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->mean);

    model->stdDev = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->stdDev);
}

void initHistogramModel(HistogramModel* model, CvSize size)
{
    model->height = size.height;
    model->width = size.width;

    // Alloue un histogramme pour chaque pixel
    model->hist = (Histogram**)malloc(size.height*sizeof(Histogram*));

    int row, col;
    for(row = 0; row < size.height; row++)
    {
        model->hist[row] = (Histogram*)malloc(size.width*sizeof(Histogram));
        
        for(col = 0; col < size.width; col++)
            initHistogram(&(model->hist[row][col]), 256, 3); 
    }
}

void releaseHistogramModel(HistogramModel* model)
{
    int row,col;
    for(row = 0; row < model->height; row++)
    {
        for(col = 0; col < model->width; col++)
        {
            releaseHistogram(&model->hist[row][col]);
        }
        free(model->hist[row]);
    }
    free(model->hist);
}

void releaseMedianModel(MedianModel* model)
{
    if(model->median != NULL)
        cvReleaseImage(&model->median);
    if(model->high != NULL)
        cvReleaseImage(&model->high);
    if(model->low != NULL)
        cvReleaseImage(&model->low);
    if(model->stdDev != NULL)
        cvReleaseImage(&model->stdDev);
}

void releaseGaussianModel(GaussianModel* model)
{
    if(model->mean != NULL)
        cvReleaseImage(&model->mean);
    if(model->stdDev != NULL)
        cvReleaseImage(&model->stdDev);
}

void learnHistogramModel(HistogramModel* model, char* dir, char* pattern, int frameStart, int frameEnd)
{
    CvSize fSize = cvSize(0, 0);
    IplImage* f = NULL;
    char filename[256];
    int i;
    for(i = frameStart; i < frameEnd; i++)
    {
        sprintf(filename, pattern, dir, i);

        f = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(f == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        if(fSize.height == 0 && fSize.width == 0)
        {
            printf("Init model");

            fSize = cvGetSize(f);
            initHistogramModel(model, fSize);
        }

        // Mise a jour du modele pixel par pixel
        int row, col;
        for(row = 0; row < fSize.height; row++)
        {
            for(col = 0; col < fSize.width; col++)
            {
                updateHistogram(&model->hist[row][col], f, col, row);
            }
        }

        cvReleaseImage(&f);
    }
}

void learnMedianModel(MedianModel* model, char* dir, char* pattern, int frameCount, float percentile)
{
    char filename[255];

    // Determine la taille des images
    sprintf(filename, pattern, dir, 0);
    IplImage* firstFrame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    if(firstFrame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
        return;
    }
    CvSize fSize = cvGetSize(firstFrame);   
    cvReleaseImage(&firstFrame);

    initMedianModel(model, fSize, percentile);
    IplImage *f; 
    IplImage *median = model->median, *pHigh = model->high, *pLow = model->low;
    IplImage* sd = model->stdDev;

    uchar pixelsBlue[frameCount], pixelsGreen[frameCount], pixelsRed[frameCount];
    float medianBlue, medianGreen, medianRed;
    float pctHighBlue, pctHighGreen, pctHighRed;
    float pctLowBlue, pctLowGreen, pctLowRed;
    float sdvBlue, sdvGreen, sdvRed;
    int step = median->widthStep, iStep;
    int row, col, i;
    for(row = 0; row < fSize.height; row++)
    {
        for(col = 0; col < fSize.width; col++)
        {
            for(i = 0; i < frameCount; i++)
            {
                sprintf(filename, pattern, dir, i);

                f = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
                if(f == NULL)
                {
                    fprintf(stderr, "Erreur de lecture de l'image %s\n", 
                            filename);
                    continue;
                }

                iStep = f->widthStep;

                pixelsBlue[i] = ((uchar*)(f->imageData + iStep*row))[col*3];
                pixelsGreen[i] = ((uchar*)(f->imageData + iStep*row))[col*3+1];
                pixelsRed[i] = ((uchar*)(f->imageData + iStep*row))[col*3+2];

                cvReleaseImage(&f);
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

            // Calcul des ecart-types
            computeMeanSdv(pixelsBlue, frameCount, NULL, &sdvBlue);
            computeMeanSdv(pixelsGreen, frameCount, NULL, &sdvGreen);
            computeMeanSdv(pixelsRed, frameCount, NULL, &sdvRed);

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

            // Ecart-types
            ((float*)(sd->imageData + step*row))[col*3] = sdvBlue;
            ((float*)(sd->imageData + step*row))[col*3+1] = sdvGreen;
            ((float*)(sd->imageData + step*row))[col*3+2] = sdvRed;
        }
    }
}

void learnAdaptiveMedianModel(MedianModel* model, char* dir, char* pattern, int frameStart, int frameEnd, float percentile)
{
    CvSize fSize = cvSize(0,0);

    IplImage *f, *pf = NULL, *ppf = NULL; 
    IplImage *median, *sd;
    IplImage* stdDev;

    uchar blue, green, red;
    uchar pblue, pgreen, pred;
    float medianBlue, medianGreen, medianRed;
    float sdvBlue = 0.0, sdvGreen = 0.0, sdvRed = 0.0;
    int iStep, stepm;
    int row, col, i;
    int frameCount = 0;
    
    float sum = 0.0, diff = 0.0;

    char filename[255];
    for(i = frameStart; i < frameEnd; i++)
    {
        sprintf(filename, pattern, dir, i);

        f = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(f == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        // Initialisation du modele avec la premiere frame
        if(i == 0)
        {
            fSize = cvGetSize(f);
            initMedianModel(model, fSize, percentile);
            cvCvtScale(f, model->median, 1.0, 0);
            stdDev = cvCreateImage(fSize, IPL_DEPTH_32F, 3);

            median = model->median;
            sd = model->stdDev;
            stepm = median->widthStep;
        }

        // Mise a jour du modele pixel par pixel
        int row, col, iStep;
        for(row = 0; row < fSize.height; row++)
        {
            for(col = 0; col < fSize.width; col++)
            {
                iStep = f->widthStep;

                // Valeurs courantes de medianne
                medianBlue = ((float*)(median->imageData + stepm*row))[col*3];
                medianGreen= ((float*)(median->imageData + stepm*row))[col*3+1];
                medianRed = ((float*)(median->imageData + stepm*row))[col*3+2];

                // Nouvelles valeurs de pixels
                blue = ((uchar*)(f->imageData + iStep*row))[col*3];
                green = ((uchar*)(f->imageData + iStep*row))[col*3+1];
                red = ((uchar*)(f->imageData + iStep*row))[col*3+2];
                
                // Mise a jour des valeurs mediannes
                if(blue > medianBlue)
                    medianBlue++;
                else
                    medianBlue--;

                if(green > medianGreen)
                    medianGreen++;
                else
                    medianGreen--;

                if(red > medianRed)
                    medianRed++;
                else
                    medianRed--;

                // Positionne les nouvelles valeurs de mediannes dans le modele
                ((float*)(median->imageData + stepm*row))[col*3] = medianBlue;
                ((float*)(median->imageData + stepm*row))[col*3+1]= medianGreen;
                ((float*)(median->imageData + stepm*row))[col*3+2] = medianRed;

                if(i > 0)
                {
                    // Valeurs courantes d'ecart-type
                    sdvBlue = ((float*)(sd->imageData + stepm*row))[col*3];
                    sdvGreen= ((float*)(sd->imageData + stepm*row))[col*3+1];
                    sdvRed = ((float*)(sd->imageData + stepm*row))[col*3+2];

                    // Valeurs precedentes d'image
                    pblue = ((uchar*)(pf->imageData + iStep*row))[col*3];
                    pgreen = ((uchar*)(pf->imageData + iStep*row))[col*3+1];
                    pred = ((uchar*)(pf->imageData + iStep*row))[col*3+2];
                    
                    // Differences d'images
                    sdvBlue += fabsf(blue - pblue);
                    sdvGreen += fabsf(green - pgreen);
                    sdvRed += fabsf(red - pred);

                    // Positionne les nouvelles valeurs
                    ((float*)(sd->imageData + stepm*row))[col*3] = sdvBlue;
                    ((float*)(sd->imageData + stepm*row))[col*3+1] = sdvGreen;
                    ((float*)(sd->imageData + stepm*row))[col*3+2] = sdvRed;
                }
            }
        }
        ppf = pf;
        pf = f;

        if(ppf != NULL)
            cvReleaseImage(&ppf);

        frameCount++;
    }

    // Moyenne des differences ~= ecart-types
    //cvCvtScale(sd, sd, (float)(1.0/frameCount), 0);

    //cvReleaseImage(&f);
}

void learnRunningMedianModel(MedianModel* model, IplImage* frameBuffer[], int frameCount, float percentile)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);

    initMedianModel(model, fSize, percentile);
    IplImage *f; 
    IplImage *median = model->median;

    uchar blue, green, red;
    float medianBlue, medianGreen, medianRed;
    int iStep, stepm = median->widthStep;
    int row, col, i;

    for(i = 0; i < frameCount; i++)
    {
        f = frameBuffer[i];

        if(i == 0)
        {
            cvCvtScale(f, model->median, 1.0, 0);
        }

        // Mise a jour du modele pixel par pixel
        int row, col, iStep;
        for(row = 0; row < fSize.height; row++)
        {
            for(col = 0; col < fSize.width; col++)
            {
                iStep = f->widthStep;

                // Valeurs courantes de medianne
                medianBlue = ((float*)(median->imageData + stepm*row))[col*3];
                medianGreen= ((float*)(median->imageData + stepm*row))[col*3+1];
                medianRed = ((float*)(median->imageData + stepm*row))[col*3+2];

                // Nouvelles valeurs de pixels
                blue = ((uchar*)(f->imageData + iStep*row))[col*3];
                green = ((uchar*)(f->imageData + iStep*row))[col*3+1];
                red = ((uchar*)(f->imageData + iStep*row))[col*3+2];
                
                // Mise a jour des valeurs mediannes
                if(blue > medianBlue)
                    medianBlue++;
                else
                    medianBlue--;

                if(green > medianGreen)
                    medianGreen++;
                else
                    medianGreen--;

                if(red > medianRed)
                    medianRed++;
                else
                    medianRed--;

                // Positionne les nouvelles valeurs de mediannes dans le modele
                ((float*)(median->imageData + stepm*row))[col*3] = medianBlue;
                ((float*)(median->imageData + stepm*row))[col*3+1]= medianGreen;
                ((float*)(median->imageData + stepm*row))[col*3+2] = medianRed;
            }
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
    uchar blue, green, red;
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
        cvReleaseImage(&f);
    }
}

IplImage* segmentMedian(IplImage* frame, float threshold, MedianModel* mm)
{
    // Images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrdA = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
    IplImage* foregrdB = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdG = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdR = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Applique la regle: si |P - M_P| > threshold => (P == foreground)
    // sur chaque plan
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, mm->median, diff);

    cvCvtScale(diff, foregrdA, 1, 0);
    cvThreshold(foregrdA, foregrdA, threshold, 255, CV_THRESH_BINARY);

    // Combine l'information des 3 plans avec une operation OR
    cvSplit(foregrdA, foregrdB, foregrdG, foregrdR, NULL);
    cvOr(foregrdB, foregrdG, foregrd, NULL);
    cvOr(foregrd, foregrdR, foregrd, NULL);

    cvReleaseImage(&frameF);
    cvReleaseImage(&diff);
    cvReleaseImage(&foregrdA);
    cvReleaseImage(&foregrdB);
    cvReleaseImage(&foregrdG);
    cvReleaseImage(&foregrdR);

    return foregrd;
}

IplImage* segmentMedianStdDev(IplImage* frame, int k, MedianModel* mm)
{
    // Images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* threshold = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrdF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* foregrdA = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
    IplImage* foregrdB = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdG = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdR = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Applique la regle: si |P - M_P| > k * sigma => (P == foreground)
    // sur chaque plan
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, mm->median, diff);

    // Creation de l'image seuil definie en fonction de l'ecart-type
    cvScale(mm->stdDev, threshold, k, 0.0);

    // Les differences au dela de l'image seuil (positives) sont l'avant-plan
    cvSub(diff, threshold, foregrdF, NULL);
    cvThreshold(foregrdF, foregrdF, 0.0, 255, CV_THRESH_BINARY);
    cvScale(foregrdF, foregrdA, 1, 0);

    // Combine l'information des 3 plans avec une operation OR
    cvSplit(foregrdA, foregrdB, foregrdG, foregrdR, NULL);
    cvOr(foregrdB, foregrdG, foregrd, NULL);
    cvOr(foregrd, foregrdR, foregrd, NULL);

    cvReleaseImage(&frameF);
    cvReleaseImage(&diff);
    cvReleaseImage(&threshold);
    cvReleaseImage(&foregrdF);
    cvReleaseImage(&foregrdA);
    cvReleaseImage(&foregrdB);
    cvReleaseImage(&foregrdG);
    cvReleaseImage(&foregrdR);

    return foregrd;
}

IplImage* segmentPercentile(IplImage* frame, MedianModel* mm)
{
    // Images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* diffH = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* diffL = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrdF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);

    cvCvtScale(frame, frameF, 1, 0);

    // On met a 255 les differences positives (tombent dans poucentile extreme)
    cvSub(frameF, mm->high, diffH, NULL);
    cvThreshold(diffH, diffH, 0.0, 255, CV_THRESH_BINARY);

    cvSub(mm->low, frameF, diffL, NULL);
    cvThreshold(diffL, diffL, 0.0, 255, CV_THRESH_BINARY);

    cvOr(diffH, diffL, foregrdF, NULL);
    cvScale(foregrdF, foregrd, 1, 0);

    cvReleaseImage(&diffH);
    cvReleaseImage(&diffL);
    cvReleaseImage(&foregrdF);

    return foregrd;
}

IplImage* segmentGaussian(IplImage* frame, float k, GaussianModel* gm)
{
    // Images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* threshold = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrdF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* foregrdA = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
    IplImage* foregrdB = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdG = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdR = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Applique la regle: si |P - M_P| > k * sigma => (P == foreground)
    // sur chaque plan
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, gm->mean, diff);

    // Creation de l'image seuil definie en fonction de l'ecart-type
    cvScale(gm->stdDev, threshold, k, 0.0);

    // Les differences au dela de l'image seuil (positives) sont l'avant-plan
    cvSub(diff, threshold, foregrdF, NULL);
    cvThreshold(foregrdF, foregrdF, 0.0, 255, CV_THRESH_BINARY);
    cvScale(foregrdF, foregrdA, 1, 0);

    // Combine l'information des 3 plans avec une operation OR
    cvSplit(foregrdA, foregrdB, foregrdG, foregrdR, NULL);
    cvOr(foregrdB, foregrdG, foregrd, NULL);
    cvOr(foregrd, foregrdR, foregrd, NULL);

    cvReleaseImage(&frameF);
    cvReleaseImage(&diff);
    cvReleaseImage(&threshold);
    cvReleaseImage(&foregrdF);
    cvReleaseImage(&foregrdA);
    cvReleaseImage(&foregrdB);
    cvReleaseImage(&foregrdG);
    cvReleaseImage(&foregrdR);

    return foregrd;
}

IplImage* segmentMean(IplImage* frame, float threshold, GaussianModel* gm)
{
    printFrame(gm->mean, 0, "meanBlue.csv");
    printFrame(gm->mean, 1, "meanGreen.csv");
    printFrame(gm->mean, 2, "meanRed.csv");

    // Images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrdF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* foregrdA = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
    IplImage* foregrdB = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdG = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdR = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Applique la regle: si |P - M_P| > k * sigma => (P == foreground)
    // sur chaque plan
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, gm->mean, diff);

    cvCvtScale(diff, foregrdA, 1, 0);
    cvThreshold(foregrdA, foregrdA, threshold, 255, CV_THRESH_BINARY);

    // Combine l'information des 3 plans avec une operation OR
    cvSplit(foregrdA, foregrdB, foregrdG, foregrdR, NULL);
    cvOr(foregrdB, foregrdG, foregrd, NULL);
    cvOr(foregrd, foregrdR, foregrd, NULL);

    return foregrd;
}

IplImage* stdDevImage(float k, GaussianModel* gm)
{
    CvSize size = cvGetSize(gm->mean);

    // Images intermediaires
    IplImage* foregrdF = cvCreateImage(size, IPL_DEPTH_32F, 3); 
    IplImage* foregrdA = cvCreateImage(size, IPL_DEPTH_8U, 3);
    IplImage* foregrdB = cvCreateImage(size, IPL_DEPTH_8U, 1);
    IplImage* foregrdG = cvCreateImage(size, IPL_DEPTH_8U, 1);
    IplImage* foregrdR = cvCreateImage(size, IPL_DEPTH_8U, 1);

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(size, IPL_DEPTH_8U, 1);

    cvThreshold(gm->stdDev, foregrdF, k, 255, CV_THRESH_BINARY);
    cvCvtScale(foregrdF, foregrdA, 1, 0);
    
    // Combine l'information des 3 plans avec une operation OR
    cvSplit(foregrdA, foregrdB, foregrdG, foregrdR, NULL);
    cvOr(foregrdB, foregrdG, foregrd, NULL);
    cvOr(foregrd, foregrdR, foregrd, NULL);

    return foregrd;
}
