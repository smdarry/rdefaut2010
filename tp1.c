#include "cv.h"
#include "highgui.h"

#include <stdio.h>

#include "histogram.h"
#include "models.h"

#define IMAGE_COUNT 795
#define FRAME_BUF_SIZE 10
#define FRAME_SAMPLING 10

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
