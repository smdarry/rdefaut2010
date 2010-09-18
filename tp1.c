#include "cv.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>

#include "histogram.h"
#include "models.h"

#define IMAGE_COUNT 795
#define FRAME_BUF_SIZE 10
#define FRAME_SAMPLING 10

int cmpPixel(const void *p1, const void *p2)
{
    return *((const char*)p1) - *((const char*)p2);
}

float computeMedian(char pixelArray[], int size)
{
    float median;
    int size_2 = size / 2;

    qsort(pixelArray, size, sizeof(uchar), cmpPixel);
    if(size % 2 == 0)
        median = (pixelArray[size_2-1] + pixelArray[size_2]) / 2.0;
    else
        median = pixelArray[size_2];
    
    return median;
}

void computeMeanSdv(char pixelArray[], int size, float* mean, float* sdv)
{
    *mean = 0.0, *sdv = 0.0;
    
    // Moyenne
    int i;
    for(i = 0; i < size; i++)
    {
        *mean += pixelArray[i];
    }
    *mean /= size;

    // et l'ecart-type...
    if(sdv == NULL) return;

    float sdiff;
    for(i = 0; i < size; i++)
    {
        sdiff = pixelArray[i] - *mean;
        *sdv += sdiff * sdiff;
    }
    *sdv = sqrt(*sdv / size);
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

void learnModels(char* directory, MedianModel* mm, GaussianModel* gm)
{
    IplImage* frame = NULL;

    // Un buffer circulaire permet de ne garder que les n dernieres frames
    IplImage* frameBuffer[FRAME_BUF_SIZE];
    int frameBufIndex = 0;

    int i;
    for(i = 0; i < FRAME_BUF_SIZE; i++)
    {
        frameBuffer[i] = NULL;
    }

    char filename[256];
    CvSize frameSize = cvSize(0, 0);

    for(i = 0; i < IMAGE_COUNT; i += FRAME_SAMPLING)
    {
        sprintf(filename, "%s/frame_%04d.jpg", directory, i);

        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(frame == NULL)
        {
            fprintf(stdout, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        if(frameSize.width == 0 && frameSize.height == 0)
            frameSize = cvSize(frame->width, frame->height);
        
        // Garde l'image dans le buffer circulaire
        frameBufIndex = (frameBufIndex + 1) % FRAME_BUF_SIZE;
        IplImage* outdatedFrame = frameBuffer[frameBufIndex];
        if(outdatedFrame != NULL)
            cvReleaseImage(&outdatedFrame);
        frameBuffer[frameBufIndex] = frame;
    }
    
    learnMedianModel(mm, frameBuffer, FRAME_BUF_SIZE);
    learnGaussianModel(gm, frameBuffer, FRAME_BUF_SIZE);
}

IplImage* segmentMedian(IplImage* frame, float thresh, MedianModel* mm)
{
    // Allocation des images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);

    // Applique la regle de decision
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, mm->median, diff);

    cvCvtScale(diff, foregrd, 1, 0);
    cvThreshold(foregrd, foregrd, thresh, 255, CV_THRESH_BINARY_INV);

    cvReleaseImage(&frameF);
    cvReleaseImage(&diff);

    return foregrd;
}

IplImage* segmentGaussian(IplImage* frame, float thresh, GaussianModel* gm)
{
    // Allocation des images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);

    // Applique la regle de decision
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, gm->mean, diff);

    cvCvtScale(diff, foregrd, 1, 0);
    cvThreshold(foregrd, foregrd, thresh, 255, CV_THRESH_BINARY_INV);

    cvReleaseImage(&frameF);
    cvReleaseImage(&diff);

    return foregrd;
}

int main( int argc, char** argv )
{
    ////////////////////////////////////////
    // Question 2: etude des modeles de fond

    MedianModel medianModel;
    GaussianModel gaussianModel;

    learnModels("../View_008", &medianModel, &gaussianModel);


    /////////////////////////////////////////////////////////
    // Question 3: etude du modele de decision (segmentation)
    // Charge l'image a segmenter
    const char* filename = "../View_008/frame_0189.jpg";
    IplImage* frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
    }

    IplImage* forMedian = segmentMedian(frame, 20.0, &medianModel);
    IplImage* forGauss = segmentGaussian(frame, 20.0, &gaussianModel);

    cvNamedWindow("Foreground - Median", CV_WINDOW_AUTOSIZE);
    cvShowImage("Foreground - Median", forMedian);

    cvNamedWindow("Foreground - Gaussian", CV_WINDOW_AUTOSIZE);
    cvShowImage("Foreground - Gaussian", forGauss);

    cvWaitKey(0);

    cvReleaseImage(&frame);
    cvReleaseImage(&forMedian);
    cvReleaseImage(&forGauss);

    releaseMedianModel(&medianModel);
    releaseGaussianModel(&gaussianModel);
}
