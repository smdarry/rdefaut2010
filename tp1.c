#include "cv.h"
#include "highgui.h"

#include "histogram.h"
#include "models.h"

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
    for(i = 0; i < frameCount; i += interval)
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
        IplImage* outdatedFrame = frameBuffer[frameBufIndex];
        if(outdatedFrame != NULL)
            cvReleaseImage(&outdatedFrame);
        frameBuffer[frameBufIndex] = frame;
        frameBufIndex = (frameBufIndex + 1) % frameCount;
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

IplImage* segmentStdDev(IplImage* frame, float k, GaussianModel* gm)
{
    printFrame(gm->stdDev, 0, "stdBlue.csv");
    printFrame(gm->stdDev, 1, "stdGreen.csv");
    printFrame(gm->stdDev, 2, "stdRed.csv");

    // Images intermediaires
    IplImage* foregrdF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* foregrdA = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
    IplImage* foregrdB = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdG = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage* foregrdR = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    cvThreshold(gm->stdDev, foregrdF, k, 255, CV_THRESH_BINARY);
    cvCvtScale(foregrdF, foregrdA, 1, 0);
    
    // Combine l'information des 3 plans avec une operation OR
    cvSplit(foregrdA, foregrdB, foregrdG, foregrdR, NULL);
    cvOr(foregrdB, foregrdG, foregrd, NULL);
    cvOr(foregrd, foregrdR, foregrd, NULL);

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

IplImage* segmentGaussian(IplImage* frame, float k, GaussianModel* gm)
{
    IplImage* meanImage = segmentMean(frame, 40.0, gm);
    IplImage* stdImage = segmentStdDev(frame, 9.0, gm);

    // Image binaire resultante
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    // Combine l'information des deux images par une operation AND
    cvAnd(meanImage, stdImage, foregrd, NULL);
    
    return foregrd;
}

void computePixelStatistics(char* dir, int imageCount)
{
    // Histogrammes temporels pour 3 pixels
    Histogram h1, h2, h3;
    initHistogram(&h1);
    initHistogram(&h2);
    initHistogram(&h3);

    // Chronogrammes pour 3 pixels
    CvMat *chrono1 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono2 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono3 = cvCreateMat(1, imageCount, CV_8UC3);

    CvMat *chrono4 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono5 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono6 = cvCreateMat(1, imageCount, CV_8UC3);

    char filename[256];
    IplImage* frame = NULL;

    int i;
    for(i = 0; i < imageCount; i++)
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

        updateChronogram(chrono1, frame, i, 10, 10);
        updateChronogram(chrono2, frame, i, 596, 265);
        updateChronogram(chrono3, frame, i, 217, 137);

        updateChronogram(chrono4, frame, i, 109, 167);
        //updateChronogram(chrono5, frame, i, 596, 265);
        //updateChronogram(chrono6, frame, i, 217, 137);

        cvReleaseImage(&frame);
    }

    writeHistogram(&h1, "output/hist_10x10.csv");
    writeHistogram(&h2, "output/hist_596x265.csv");
    writeHistogram(&h3, "output/hist_217x137.csv");

    writeChronogram(chrono1, "output/chrono_10x10.csv");
    writeChronogram(chrono2, "output/chrono_596x265.csv");
    writeChronogram(chrono3, "output/chrono_217x137.csv");

    writeChronogram(chrono4, "output/chrono_109x167.csv");
    //writeChronogram(chrono5, "output/chrono_596x265.csv");
    //writeChronogram(chrono6, "output/chrono_109x167.csv");

    cvReleaseMat(&chrono1);
    cvReleaseMat(&chrono2);
    cvReleaseMat(&chrono3);

    cvReleaseMat(&chrono4);
    cvReleaseMat(&chrono5);
    cvReleaseMat(&chrono6);
}

int main( int argc, char** argv )
{
    int imageCount = 100;//IMAGE_COUNT;

    ////////////////////////////////////////
    // Question 1: problematique de la segmentation

    // Tracage des histogrammes temporels et chronogrammes pour 3 pixels
    //computePixelStatistics("../View_008", imageCount);
    //computePixelStatistics("../View_008", 70);


    ////////////////////////////////////////
    // Question 2: etude des modeles de fond

    MedianModel medianModel;
    GaussianModel gaussianModel;
    GaussianModel adaptiveGaussianModel;

    IplImage* frameBuffer[imageCount];

    // Prend un echantillon d'images
    selectFrames("../View_008", frameBuffer, imageCount, 1);

    // Construction des modeles
    learnMedianModel(&medianModel, frameBuffer, imageCount);
    learnGaussianModel(&gaussianModel, frameBuffer, imageCount);
    learnAdaptiveGaussian(&adaptiveGaussianModel, "../View_008", 0.5, imageCount, 1);


    /////////////////////////////////////////////////////////
    // Question 3: etude du modele de decision (segmentation)
    
    const char* toSegment = "../View_008/frame_0061.jpg";
    IplImage* frame = cvLoadImage(toSegment, CV_LOAD_IMAGE_COLOR);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", toSegment);
    }

    IplImage* forMedian = segmentMedian(frame, 40.0, &medianModel);
    IplImage* forGauss = segmentGaussian(frame, 3.0, &gaussianModel);
    IplImage* forMean = segmentMean(frame, 50.0, &gaussianModel);
    IplImage* forAdaMean = segmentMean(frame, 40.0, &adaptiveGaussianModel);

    cvSaveImage("Median.jpg", forMedian);
    cvSaveImage("Gaussian.jpg", forGauss);
    cvSaveImage("Mean.jpg", forMean);
    cvSaveImage("MeanAdaptive.jpg", forAdaMean);

/*
    // Pixels d'arriere-plan
    
    printf("\n==> Pixels d'arriere-plan\n");

    // Brique et feuillage dans le coin superieur gauche
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 10, 10);

    // Ciment a droite des jambes de la 5e personne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 596, 265);

    // Ciment a droite des jambes de la 2e personne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 325, 225);

    // Ciment a gauche du corps de la 1ere personne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 150, 180);

    // Brique au dessus de la 3e personne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 360, 77);

    // Bruit dans l'image moyenne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 109, 167);


    // Pixels d'avant-plan
    
    printf("\n==> Pixels d'avant-plan\n");

    // Manteau de la 1ere personne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 217, 137);

    // Manteau de la 2e personne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 312, 151);

    // Manteau de la 5e personne
    analysePixel(frame, gaussianModel.mean, gaussianModel.stdDev, 557, 170);

    //cvWaitKey(0);
*/

    ////////////////////////////////////////////////////////
    // Question 4: etude de la mise a jour de l'arriere-plan

    releaseMedianModel(&medianModel);
    releaseGaussianModel(&gaussianModel);
    releaseGaussianModel(&adaptiveGaussianModel);
/*
    // Question 5: nettoyage du resultat de la segmentation
    
    // a) une ouverture seule
    //opening(forMedian, forMedian, 3);
    
    // b) une ouverture puis une fermeture
    opening(forMedian, forMedian, 3);
    closing(forMedian, forMedian, 3);

    cvNamedWindow("Foreground - Median", CV_WINDOW_AUTOSIZE);
    cvShowImage("Foreground - Median", forMedian);
    
    cvWaitKey(0);

    cvReleaseImage(&frame);
    cvReleaseImage(&forMedian);
    cvReleaseImage(&forGauss);
*/
}
