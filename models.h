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

void learnMedianModel(MedianModel* model, IplImage* frameBuffer[], int frameCount)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);

    initMedianModel(model, fSize);

    uchar* pixel;
    char pixelsBlue[frameCount], pixelsGreen[frameCount], pixelsRed[frameCount];
    float medianBlue, medianGreen, medianRed;
    float* data = (float*)model->median->imageData;
    int row, col, i, step = model->median->widthStep;
    for(row = 0; row < fSize.height; row++)
    {
        for(col = 0; col < fSize.width; col++)
        {
            for(i = 0; i < frameCount; i++)
            {
                pixel = (uchar*)GET_PTR_AT(frameBuffer[i], col, row);

                pixelsBlue[i] = *pixel;
                pixelsGreen[i] = *(pixel+1);
                pixelsRed[i] = *(pixel+2);
            }

            // Calcul des mediannes
            medianBlue = computeMedian(pixelsBlue, frameCount);
            medianGreen = computeMedian(pixelsGreen, frameCount);
            medianRed = computeMedian(pixelsRed, frameCount);

            // Positionne les valeurs mediannes dans le modele
            ((float*)(data + step*row))[col*3] = medianBlue; 
            ((float*)(data + step*row))[col*3 + 1] = medianGreen; 
            ((float*)(data + step*row))[col*3 + 2] = medianRed; 
        }
    }
}

void learnGaussianModel(GaussianModel* model, IplImage* frameBuffer[], int frameCount)
{
    CvSize fSize = cvGetSize(frameBuffer[0]);

    initGaussianModel(model, fSize);

    uchar* pixel;
    uchar pixelsBlue[frameCount], pixelsGreen[frameCount], 
          pixelsRed[frameCount];
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
                pixel = (uchar*)GET_PTR_AT(frameBuffer[i], col, row);

                pixelsBlue[i] = *pixel;
                pixelsGreen[i] = *(pixel+1);
                pixelsRed[i] = *(pixel+2);
            }
            
            // Calcul des parametres gaussiens
            computeMeanSdv(pixelsBlue, frameCount, &meanBlue, &sdvBlue);
            computeMeanSdv(pixelsGreen, frameCount, &meanGreen, &sdvGreen);
            computeMeanSdv(pixelsRed, frameCount, &meanRed, &sdvRed);

            // Positionne les valeurs de moyenne dans le modele
            float* ptr = (float*)(model->mean->imageData + fSize.width*row*3 + col*3); 

            *ptr = meanBlue;
            *(ptr+1) = meanGreen;
            *(ptr+2) = meanRed;

            // Positionne les valeurs d'ecart-type dans le modele
            ptr = (float*)(model->stdDev->imageData + fSize.width*row*3 + col*3); 

            *ptr = sdvBlue;
            *(ptr+1) = sdvGreen;
            *(ptr+2) = sdvRed;
        }
    }
}
