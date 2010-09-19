#include "cv.h"
#include "highgui.h"

#include "histogram.h"
#include "models.h"
#include "utils.h"

#define IMAGE_COUNT 795
#define FRAME_BUF_SIZE 10
#define FRAME_SAMPLING 10

/**
 * TODO: Fonctionne sous la supposition que le frameBuffer sera toujours plein.
 */
void selectFrames(char* dir, IplImage* frameBuffer[], int frameCount, int interval)
{
    // Un buffer circulaire permet de ne garder que les n dernieres frames
    int frameBufIndex = 0;

    // Initialize les pointeurs d'image a NULL
    int i;
    for(i = 0; i < frameCount; i++)
    {
        frameBuffer[i] = NULL;
    }

    char filename[256];
    CvSize frameSize = cvSize(0, 0);

    IplImage* frame = NULL;
    for(i = 0; i < IMAGE_COUNT; i += interval)
    {
        sprintf(filename, "%s/frame_%04d.jpg", dir, i);

        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(frame == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        // Garde l'image selectionnee dans le buffer circulaire
        // en prenant soin de liberer l'image qui se fera bumper
        frameBufIndex = (frameBufIndex + 1) % frameCount;
        IplImage* outdatedFrame = frameBuffer[frameBufIndex];
        if(outdatedFrame != NULL)
            cvReleaseImage(&outdatedFrame);
        frameBuffer[frameBufIndex] = frame;
    }
}

IplImage* segmentMedian(IplImage* frame, float threshold, MedianModel* mm)
{
    // Allocation des images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);

    IplImage* foregrdA = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
    IplImage* foregrdB = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdG = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdR = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Applique la regle: si |P - M_P| > threshold => (P == foreground)
    // sur chaque plan
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, mm->median, diff);

    cvCvtScale(diff, foregrdA, 1, 0);
    cvThreshold(foregrdA, foregrdA, threshold, 255, CV_THRESH_BINARY_INV);

    // Combine l'information des 3 plans avec une operation OR
    cvSplit(foregrdA, foregrdB, foregrdG, foregrdR, NULL);
    cvOr(foregrdB, foregrdG, foregrd, NULL);
    cvOr(foregrd, foregrdR, foregrd, NULL);

    cvReleaseImage(&frameF);
    cvReleaseImage(&diff);

    return foregrd;
}

IplImage* segmentGaussian(IplImage* frame, float k, GaussianModel* gm)
{
    printByteFrame(frame, 0, "frame.csv");
    printFrame(gm->mean, 0, "meanMatrix.csv");
    printFrame(gm->stdDev, 0, "stdDevMatrix.csv");

    // Allocation des images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* threshold = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrdF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);

    // Applique la regle: si |P - M_P| > k * sigma => (P == foreground)
    // sur chaque plan
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, gm->mean, diff);

    cvScale(gm->stdDev, threshold, k, 0.0);
    cvSub(diff, threshold, foregrdF, NULL);
    cvThreshold(foregrdF, foregrdF, 0.0, 255, CV_THRESH_BINARY);
    cvScale(foregrdF, foregrd, 1, 0);

    cvReleaseImage(&frameF);
    cvReleaseImage(&diff);
    cvReleaseImage(&threshold);
    cvReleaseImage(&foregrdF);

    return foregrd;
}

void computeHistograms(char* dir)
{
    Histogram h1, h2, h3;
    initHistogram(&h1);
    initHistogram(&h2);
    initHistogram(&h3);

    char filename[256];
    IplImage* frame = NULL;
    int i;
    for(i = 0; i < IMAGE_COUNT; i++)
    {
        sprintf(filename, "%s/frame_%04d.jpg", dir, i);

        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(frame == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        updateHistogram(&h1, frame, 10, 10);
        updateHistogram(&h2, frame, 596, 265);
        updateHistogram(&h3, frame, 217, 137);

        cvReleaseImage(&frame);
    }

    writeHistogram(&h1, "hist_10x10.csv");
    writeHistogram(&h2, "hist_596x265.csv");
    writeHistogram(&h3, "hist_217x137.csv");
}

int main( int argc, char** argv )
{
    ////////////////////////////////////////
    // Question 1: problematique de la segmentation

    // Tracage des histogrammes temporels pour 3 pixels
    computeHistograms("../View_008");


    ////////////////////////////////////////
    // Question 2: etude des modeles de fond

    MedianModel medianModel;
    GaussianModel gaussianModel;

    // Construction du modele median
    IplImage* frameBuffer[FRAME_BUF_SIZE];
    selectFrames("../View_008", frameBuffer, FRAME_BUF_SIZE, FRAME_SAMPLING);
    learnMedianModel(&medianModel, frameBuffer, FRAME_BUF_SIZE);

    // Construction du modele Gaussien sur le meme tableau d'images
    learnGaussianModel(&gaussianModel, frameBuffer, FRAME_BUF_SIZE);


    /////////////////////////////////////////////////////////
    // Question 3: etude du modele de decision (segmentation)
    
    const char* toSegment = "../View_008/frame_0189.jpg";
    IplImage* frame = cvLoadImage(toSegment, CV_LOAD_IMAGE_COLOR);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", toSegment);
    }

    IplImage* forMedian = segmentMedian(frame, 40.0, &medianModel);
    IplImage* forGauss = segmentGaussian(frame, 1.0, &gaussianModel);

    cvNamedWindow("Foreground - Median", CV_WINDOW_AUTOSIZE);
    cvShowImage("Foreground - Median", forMedian);

    cvNamedWindow("Foreground - Gaussian", CV_WINDOW_AUTOSIZE);
    cvShowImage("Foreground - Gaussian", forGauss);

    cvWaitKey(0);

    cvReleaseImage(&frame);
    cvReleaseImage(&forMedian);
    cvReleaseImage(&forGauss);


    ////////////////////////////////////////////////////////
    // Question 4: etude de la mise a jour de l'arriere-plan

    releaseMedianModel(&medianModel);
    releaseGaussianModel(&gaussianModel);
}
