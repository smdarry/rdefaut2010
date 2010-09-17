#include "cv.h"
#include "highgui.h"
#include "histogram.h"
#include "modeles.h"
#include <stdio.h>

#define GET_PIXEL_PTR_AT(img, row, col) img->imageData+row*3+col
#define IMAGE_COUNT 795

void updateModeleMedian(ModeleMedian* model, Histogram hist[3], IplImage* frame)
{
    if(model->median == NULL)
    {
        initModeleMedian(model, frame);
    }
}

void calculeImageMedianne(ModeleMedian* model)
{
}

void updateModeleGaussien(ModeleGaussien* model, IplImage* frame)
{
    if(model->mean == NULL)
    {
        initModeleGaussien(model, frame);
    }
}

void updateHistogramme(Histogram* h, IplImage* frame, int x, int y)
{
    // Pointer on pixel of first channel (blue)
    unsigned char* ptr;

    // Extract pixel at location [x, y]
    ptr = GET_PIXEL_PTR_AT(frame, x, y);

    // Update each histogram for that pixel
    h->freq[0][*ptr]++;     // Blue
    h->freq[1][*(ptr+1)]++; // Green
    h->freq[2][*(ptr+2)]++; // Red
}

void apprendModeles(char* directory)
{
    ModeleMedian modeleMedian;
    ModeleGaussien modeleGaussien;
    Histogram h1, h2, h3;

    initHistogram(&h1);
    initHistogram(&h2);
    initHistogram(&h3);

    IplImage* frame = NULL;
    int i;
    char filename[256];

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

        cvReleaseImage(&frame);
    }

    //updateModeleMedian(&modeleMedian, frame);
    //updateModeleGaussien(&modeleGaussien, frame);
}

void segmentation()
{
}

int main( int argc, char** argv )
{
    // Question 2: etude des modeles de fond
    apprendModeles("View_008");
}
