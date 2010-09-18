#include "cv.h"
#include "stats.h"

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
    cvReleaseImage(&model->median);
}

void releaseGaussianModel(GaussianModel* model)
{
    cvReleaseImage(&model->mean);
    cvReleaseImage(&model->stdDev);
}

void learnMedianModel(MedianModel* model, IplImage* frameBuffer[], int frameCount)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);
    int N_2 = frameCount / 2;

    initMedianModel(model, fSize);

    uchar* pixel;
    char pixelsBlue[frameCount], pixelsGreen[frameCount], pixelsRed[frameCount];
    float medianBlue, medianGreen, medianRed;
    float* ptr;
    int row, col, i;
    for(row = 0; row < fSize.height; row++)
    {
        for(col = 0; col < fSize.width; col++)
        {
            for(i = 0; i < frameCount; i++)
            {
                pixel = (uchar*)GET_PTR_AT(frameBuffer[i], row, col);

                pixelsBlue[i] = *pixel;
                pixelsGreen[i] = *(pixel+1);
                pixelsRed[i] = *(pixel+2);
            }

            // Calcul des mediannes
            medianBlue = computeMedian(pixelsBlue, frameCount);
            medianGreen = computeMedian(pixelsGreen, frameCount);
            medianRed = computeMedian(pixelsRed, frameCount);

            // Positionne les valeurs mediannes dans le modele
            ptr = (float*)GET_PTR_AT(model->median, row, col);
            *ptr = medianBlue;
            *(ptr+1) = medianGreen;
            *(ptr+2) = medianRed;
        }
    }
}

void learnGaussianModel(GaussianModel* model, IplImage* frameBuffer[], int frameCount)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);
    int N_2 = frameCount / 2;

    initGaussianModel(model, fSize);

    uchar* pixel;
    char pixelsBlue[frameCount], pixelsGreen[frameCount], pixelsRed[frameCount];
    float meanBlue, meanGreen, meanRed;
    float sdvBlue, sdvGreen, sdvRed;
    float* ptr;
    int row, col, i;
    for(row = 0; row < fSize.height; row++)
    {
        for(col = 0; col < fSize.width; col++)
        {
            for(i = 0; i < frameCount; i++)
            {
                pixel = (uchar*)GET_PTR_AT(frameBuffer[i], row, col);

                pixelsBlue[i] = *pixel;
                pixelsGreen[i] = *(pixel+1);
                pixelsRed[i] = *(pixel+2);
            }
            
            // Calcul des parametres gaussiens
            computeMeanSdv(pixelsBlue, frameCount, &meanBlue, &sdvBlue);
            computeMeanSdv(pixelsGreen, frameCount, &meanGreen, &sdvGreen);
            computeMeanSdv(pixelsRed, frameCount, &meanRed, &sdvRed);

            // Positionne les valeurs de moyenne dans le modele
            float* ptr = (float*)GET_PTR_AT(model->mean, row, col);

            *ptr = meanBlue;
            *(ptr+1) = meanGreen;
            *(ptr+2) = meanRed;

            // Positionne les valeurs d'ecart-type dans le modele
            ptr = (float*)GET_PTR_AT(model->stdDev, row, col);

            *ptr = sdvBlue;
            *(ptr+1) = sdvGreen;
            *(ptr+2) = sdvRed;
        }
    }
}
