#include "cv.h"
#include "highgui.h"
#include "histogram.h"
#include "models.h"
#include <stdio.h>

#define GET_PTR_AT(img, row, col) img->imageData+row*3+col
#define IMAGE_COUNT 795

void updateHistogram(Histogram** h, IplImage* frame, int x, int y)
{
    // Pointer on pixel of first channel (blue)
    unsigned char* ptr;

    // Extract pixel at location [x, y]
    ptr = GET_PTR_AT(frame, x, y);

    // Update each histogram for that pixel
    h[x][y].freq[0][*ptr]++;     // Blue
    h[x][y].freq[1][*(ptr+1)]++; // Green
    h[x][y].freq[2][*(ptr+2)]++; // Red
}

void learnMedianModel(MedianModel* model, Histogram** h, int x, int y)
{
    Histogram* hist = &h[x][y];
    float medianBlue = calculeMedianne(hist, 0);
    float medianGreen = calculeMedianne(hist, 1);
    float medianRed = calculeMedianne(hist, 2);
    
    float* ptr = (float*)GET_PTR_AT(model->median, x, y);

    // Positionne les valeurs mediannes dans le modele
    *ptr = medianBlue;
    *(ptr+1) = medianGreen;
    *(ptr+2) = medianRed;
}

void learnGaussianModel(GaussianModel* model, Histogram** h, int x, int y)
{
    float meanBlue, meanGreen, meanRed;
    float sdvBlue, sdvGreen, sdvRed;

    Histogram* hist = &h[x][y];
    calculeMoyEcartType(hist, &meanBlue, &sdvBlue, 0);
    calculeMoyEcartType(hist, &meanGreen, &sdvGreen, 1);
    calculeMoyEcartType(hist, &meanRed, &sdvRed, 2);

    // Positionne les valeurs de moyenne dans le modele
    float* ptr = (float*)GET_PTR_AT(model->mean, x, y);

    *ptr = meanBlue;
    *(ptr+1) = meanGreen;
    *(ptr+2) = meanRed;

    // Positionne les valeurs d'ecart-type dans le modele
    ptr = (float*)GET_PTR_AT(model->stdDev, x, y);

    *ptr = sdvBlue;
    *(ptr+1) = sdvGreen;
    *(ptr+2) = sdvRed;
}

void apprendModeles(char* directory, MedianModel* mm, GaussianModel* gm)
{
    IplImage* frame = NULL;
    int i;
    char filename[256];
    CvSize frameSize = cvSize(0, 0);
    Histogram** hist;

    for(i = 0; i < IMAGE_COUNT; i++)
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

        // Initialization des histo avec la premiere frame de la sequence
        if(i == 0)
            hist = allocHistogramMatrix(frameSize.width, frameSize.height);

        // Construit l'histogramme temporel pour chaque pixel
        int row, col;
        for(row = 0; row < frameSize.height; row++)
        {
            for(col = 0; col < frameSize.width; col++)
            {
                updateHistogram(hist, frame, row, col);
            }
        }
        cvReleaseImage(&frame);
    }

    // Apprend un modele median sur chaque pixel
    initMedianModel(mm, frameSize);
    int row, col;
    for(row = 0; row < frameSize.height; row++)
    {
        for(col = 0; col < frameSize.width; col++)
        {
            learnMedianModel(mm, hist, row, col);
        }
    }

    // Apprend un modele Gaussien sur chaque pixel
    initGaussianModel(gm, frameSize);
    for(row = 0; row < frameSize.height; row++)
    {
        for(col = 0; col < frameSize.width; col++)
        {
            learnGaussianModel(gm, hist, row, col);
        }
    }

    freeHistogramMatrix(hist, frameSize.width, frameSize.height);
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

    apprendModeles("../View_008", &medianModel, &gaussianModel);


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
