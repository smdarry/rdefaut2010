#include "cv.h"
#include "highgui.h"
#include "histogram.h"
#include "modeles.h"
#include <stdio.h>

#define GET_PTR_AT(img, row, col) img->imageData+row*3+col
#define IMAGE_COUNT 795

void updateHistogramme(Histogram* h, IplImage* frame, int x, int y)
{
    // Pointer on pixel of first channel (blue)
    unsigned char* ptr;

    // Extract pixel at location [x, y]
    ptr = GET_PTR_AT(frame, x, y);

    // Update each histogram for that pixel
    h->freq[0][*ptr]++;     // Blue
    h->freq[1][*(ptr+1)]++; // Green
    h->freq[2][*(ptr+2)]++; // Red
}

void updateModeleMedian(ModeleMedian* model, Histogram* h, int x, int y)
{
    float medianBlue = calculeMedianne(h, 0);
    float medianGreen = calculeMedianne(h, 1);
    float medianRed = calculeMedianne(h, 2);
    
    float* ptr = (float*)GET_PTR_AT(model->median, x, y);

    // Positionne les valeurs mediannes dans le modele
    *ptr = medianBlue;
    *(ptr+1) = medianGreen;
    *(ptr+2) = medianRed;
}

void updateModeleGaussien(ModeleGaussien* model, Histogram* h, int x, int y)
{
    float meanBlue, meanGreen, meanRed;
    float sdvBlue, sdvGreen, sdvRed;

    calculeMoyEcartType(h, &meanBlue, &sdvBlue, 0);
    calculeMoyEcartType(h, &meanGreen, &sdvGreen, 1);
    calculeMoyEcartType(h, &meanRed, &sdvRed, 2);

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

void apprendModeles(char* directory, ModeleMedian* mm, ModeleGaussien* mg)
{
    Histogram h1, h2, h3;

    initHistogram(&h1);
    initHistogram(&h2);
    initHistogram(&h3);

    IplImage* frame = NULL;
    int i;
    char filename[256];
    CvSize frameSize = cvSize(0, 0);

    for(i = 0; i < IMAGE_COUNT; i++)
    {
        sprintf(filename, "%s/frame_%04d.jpg", directory, i);

        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(frame == NULL)
        {
            fprintf(stdout, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        updateHistogramme(&h1, frame, 10, 10);
        updateHistogramme(&h2, frame, 596, 265);
        updateHistogramme(&h3, frame, 217, 137);

        if(frameSize.width == 0 && frameSize.height == 0)
            frameSize = cvSize(frame->width, frame->height);

        cvReleaseImage(&frame);
    }

    // Calcule un modele median sur seulement 3 pixels
    initModeleMedian(mm, frameSize);
    updateModeleMedian(mm, &h1, 10, 10);
    updateModeleMedian(mm, &h2, 596, 265);
    updateModeleMedian(mm, &h3, 217, 137);

    // Calcule un modele Gaussien sur seulement 3 pixels
    initModeleGaussien(mg, frameSize);
    updateModeleGaussien(mg, &h1, 10, 10);
    updateModeleGaussien(mg, &h2, 596, 265);
    updateModeleGaussien(mg, &h3, 217, 137);

    // TODO: Liberer l'espace des modeles
}

IplImage* segmentMedian(IplImage* frame, float thresh, ModeleMedian* mm)
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

IplImage* segmentGaussian(IplImage* frame, float thresh, ModeleGaussien* mg)
{
    // Allocation des images intermediaires
    IplImage* frameF = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3); 
    IplImage* diff = cvCreateImage(cvGetSize(frame), IPL_DEPTH_32F, 3);
    IplImage* foregrd = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);

    // Applique la regle de decision
    cvCvtScale(frame, frameF, 1, 0);
    cvAbsDiff(frameF, mg->mean, diff);

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

    ModeleMedian modeleMedian;
    ModeleGaussien modeleGaussien;

    apprendModeles("../View_008", &modeleMedian, &modeleGaussien);


    /////////////////////////////////////////////////////////
    // Question 3: etude du modele de decision (segmentation)

    // Charge l'image a segmenter
    const char* filename = "../View_008/frame_0189.jpg";
    IplImage* frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    if(frame == NULL)
    {
        fprintf(stdout, "Erreur de lecture de l'image %s\n", filename);
    }

    IplImage* forMedian = segmentMedian(frame, 20.0, &modeleMedian);
    IplImage* forGauss = segmentGaussian(frame, 20.0, &modeleGaussien);

    cvNamedWindow("Foreground - Median", CV_WINDOW_AUTOSIZE);
    cvShowImage("Foreground - Median", forMedian);

    cvNamedWindow("Foreground - Gaussian", CV_WINDOW_AUTOSIZE);
    cvShowImage("Foreground - Gaussian", forGauss);

    cvWaitKey(0);

    cvReleaseImage(&frame);
    cvReleaseImage(&forMedian);
    cvReleaseImage(&forGauss);
}
