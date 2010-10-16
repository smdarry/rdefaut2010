#include "cv.h"
#include "highgui.h"

#include "histogram.h"
#include "models.h"

#define IMAGE_COUNT 795
#define FRAME_BUF_SIZE 10
#define FRAME_SAMPLING 10

void computePixelStatistics(char* dir, int imageCount)
{
    // Histogrammes temporels pour 3 pixels
    Histogram h1, h2, h3;
    initHistogram(&h1, 256, 3);
    initHistogram(&h2, 256, 3);
    initHistogram(&h3, 256, 3);

    // Chronogrammes pour 3 pixels
    CvMat *chrono1 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono2 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono3 = cvCreateMat(1, imageCount, CV_8UC3);

    CvMat *chrono4 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono5 = cvCreateMat(1, imageCount, CV_8UC3);
    CvMat *chrono6 = cvCreateMat(1, imageCount, CV_8UC3);

    // Running means
    CvMat *rMean1 = cvCreateMat(1, imageCount, CV_32FC3);
    CvMat *rMean2 = cvCreateMat(1, imageCount, CV_32FC3);

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

     //   updateChronogram(chrono1, frame, i, 10, 10);
     //   updateChronogram(chrono2, frame, i, 596, 265);
     //   updateChronogram(chrono3, frame, i, 217, 137);

     //   updateChronogram(chrono4, frame, i, 109, 167);
     //   updateChronogram(chrono5, frame, i, 410, 120);
        updateChronogram(chrono6, frame, i, 500, 300);

//        updateRunningMean(rMean1, frame, i, 650, 160, 0.05);
        updateRunningMean(rMean2, frame, i, 500, 300, 0.05);

        cvReleaseImage(&frame);
    }

    writeHistogram(&h1, "output/hist_10x10.csv");
    writeHistogram(&h2, "output/hist_596x265.csv");
    writeHistogram(&h3, "output/hist_217x137.csv");

    //writeChronogram(chrono1, "output/chrono_10x10.csv");
    //writeChronogram(chrono2, "output/chrono_596x265.csv");
    //writeChronogram(chrono3, "output/chrono_217x137.csv");

    //writeChronogram(chrono4, "output/chrono_109x167.csv");
    //writeChronogram(chrono5, "output/chrono_410x120.csv");
    writeChronogram(chrono6, "output/chrono_500x300.csv");

 //   writeRunningMean(rMean1, "output/rmean_650x160.csv");
    writeRunningMean(rMean2, "output/rmean_500x300.csv");

    releaseHistogram(&h1);
    releaseHistogram(&h2);
    releaseHistogram(&h3);
    
    cvReleaseMat(&chrono1);
    cvReleaseMat(&chrono2);
    cvReleaseMat(&chrono3);

    cvReleaseMat(&chrono4);
    cvReleaseMat(&chrono5);
    cvReleaseMat(&chrono6);

    cvReleaseMat(&rMean1);
    cvReleaseMat(&rMean2);
}

int main( int argc, char** argv )
{
    int imageCount = IMAGE_COUNT;

/*
    ////////////////////////////////////////
    // Question 1: problematique de la segmentation

    // Tracage des histogrammes temporels et chronogrammes pour 3 pixels
    computePixelStatistics("../View_008", imageCount);
*/

/*
    ////////////////////////////////////////
    // Question 2: etude des modeles de fond

    MedianModel medianModel;
    GaussianModel gaussianModel;
    GaussianModel adaptiveGaussianModel;

    IplImage* frameBuffer[imageCount];

    // Prend un echantillon d'images
    selectFrames("../View_008", frameBuffer, imageCount, 1);

    // Construction des modeles
    learnMedianModel(&medianModel, "../View_008", "frame_%04d.jpg", imageCount, 0.95);
    learnGaussianModel(&gaussianModel, frameBuffer, imageCount);

    // Modele median incremental
    MedianModel rMedianModel;
    learnRunningMedianModel(&rMedianModel, frameBuffer, imageCount, 0.95);
    cvSaveImage("RunningMedian.jpg", rMedianModel.median);
*/

    // Modele median adaptatif
    MedianModel rMedianModel;
    learnAdaptiveMedianModel(&rMedianModel, "../View_007", "%s/1286910%d7.jpg", 87968, 88576, 0.95);
    cvSaveImage("RunningMedian.jpg", rMedianModel.median);

/*
    /////////////////////////////////////////////////////////
    // Question 3: etude du modele de decision (segmentation)
    
    const char* toSegment = "../View_008/frame_0061.jpg";
    IplImage* frame = cvLoadImage(toSegment, CV_LOAD_IMAGE_COLOR);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", toSegment);
    }

    IplImage* forMedian = segmentMedian(frame, 40.0, &medianModel);
    IplImage* forGauss = segmentGaussian(frame, 2.0, &gaussianModel);
    IplImage* forMean = segmentMean(frame, 50.0, &gaussianModel);
    IplImage* forPct = segmentPercentile(frame, &medianModel);
    IplImage* forMedianStdDev = segmentMedianStdDev(frame, 2.0, &medianModel);
    IplImage* forStdDev = stdDevImage(9.0, &gaussianModel);

    cvSaveImage("Median.jpg", forMedian);
    cvSaveImage("Gaussian.jpg", forGauss);
    cvSaveImage("Mean.jpg", forMean);
    cvSaveImage("Percentile.jpg", forPct);
    cvSaveImage("MedianStdDev.jpg", forMedianStdDev);

    releaseMedianModel(&medianModel);
*/

/*
    ////////////////////////////////////////////////////////
    // Question 4: etude de la mise a jour de l'arriere-plan
    
    const char* lastFrameName = "../View_008/frame_0794.jpg";
    IplImage* lastFrame = cvLoadImage(lastFrameName, CV_LOAD_IMAGE_COLOR);
    if(lastFrame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", lastFrameName);
    }

    learnAdaptiveGaussian(&adaptiveGaussianModel, "../View_008", 0.05, imageCount, 1);

    IplImage* forMean1 = segmentMean(lastFrame, 40.0, &gaussianModel);
    IplImage* forMean2 = segmentMean(lastFrame, 40.0, &adaptiveGaussianModel);

    cvSaveImage("Mean1.jpg", forMean1);
    cvSaveImage("Mean2.jpg", forMean2);
    
    cvSaveImage("MeanImage.jpg", gaussianModel.mean);
    cvSaveImage("AdaptiveMeanImage.jpg", adaptiveGaussianModel.mean);

    releaseGaussianModel(&adaptiveGaussianModel);
    releaseGaussianModel(&gaussianModel);

    cvReleaseImage(&lastFrame);
    cvReleaseImage(&forMean1);
    cvReleaseImage(&forMean2);
*/

/*
    ////////////////////////////////////////////////////////
    // Question 5: nettoyage du resultat de la segmentation
    
    //
    // a) une ouverture seule
    //
    // Image medianne
    openSave(forMedian, 3, "openMedian3.jpg");
    openSave(forMedian, 5, "openMedian5.jpg");

    // Image percentiles
    openSave(forPct, 3, "openPct3.jpg");
    openSave(forPct, 5, "openPct5.jpg");
    
    // Image Gaussienne simple
    openSave(forGauss, 3, "openGaussian3.jpg");
    openSave(forGauss, 5, "openGaussian5.jpg");

    // Image moyenne
    openSave(forMean, 3, "openMean3.jpg");
    openSave(forMean, 5, "openMean5.jpg");

    //
    // b) une ouverture puis une fermeture
    //
    // Image medianne
    openCloseSave(forMedian, 3, "openCloseMedian3.jpg");
    openCloseSave(forMedian, 5, "openCloseMedian5.jpg");

    // Image percentiles
    openSave(forPct, 3, "openClosePct3.jpg");
    openSave(forPct, 5, "openClosePct5.jpg");
    
    // Image Gaussienne simple
    openCloseSave(forGauss, 3, "openCloseGaussian3.jpg");
    openCloseSave(forGauss, 5, "openCloseGaussian5.jpg");

    // Image moyenne
    openCloseSave(forMean, 3, "openCloseMean3.jpg");
    openCloseSave(forMean, 5, "openCloseMean5.jpg");

    cvReleaseImage(&frame);
    cvReleaseImage(&forMedian);
    cvReleaseImage(&forGauss);
    cvReleaseImage(&forMean);
*/
}
