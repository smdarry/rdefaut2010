#include "cv.h"
#include "highgui.h"
#include <stdio.h>

#define GET_PIXEL_PTR_AT(img, row, col) img->imageData+row*3+col
#define GRAY_LEVELS 256

typedef struct _hist
{
    int _red[GRAY_LEVELS];
    int _green[GRAY_LEVELS];
    int _blue[GRAY_LEVELS];
} Histogram;

void initHistogram(Histogram* h)
{
    int i;
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        h->_red[i] = 0;
        h->_green[i] = 0;
        h->_blue[i] = 0;
    }
}

void writeHistogram(char* filename, Histogram* h)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        char error[256];
        sprintf(error, "Could not create file '%s'\n", filename);
        fprintf(stderr, error);

        return;
    }

    int i;
    for(i = 0; i < GRAY_LEVELS-1; i++)
    {
        fprintf(fp, "%d,", (int)h->_red[i]);
    }
    fprintf(fp, "%d\n", (int)h->_red[GRAY_LEVELS-1]);

    fclose(fp);
}

int main( int argc, char** argv )
{
    // 3-channel histograms for each target pixel 
    Histogram h10_10, h596_265, h217_137;

    initHistogram(&h10_10);
    initHistogram(&h596_265);
    initHistogram(&h217_137);

    int i, row;
    char filename[255];
    IplImage* img;
    for(i = 0; i < 795; i++)
    {
        sprintf(filename, "./View_008/frame_%04d.jpg", i);

        img = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(img == NULL)
        {
            fprintf(stdout, "Failed to load image %s\n", filename);
            continue;
        }

        // Pointer on pixel of first channel (blue)
        unsigned char* ptr;

        // Extract pixel at location [10, 10]
        ptr = GET_PIXEL_PTR_AT(img, 10, 10);

        // Update each histogram for that pixel
        h10_10._blue[*ptr]++;
        h10_10._green[*(ptr+1)]++;
        h10_10._red[*(ptr+2)]++;

        // Extract pixel at location [596, 265]
        ptr = GET_PIXEL_PTR_AT(img, 596, 265);

        // Update each histogram for that pixel
        h596_265._blue[*ptr]++;
        h596_265._green[*(ptr+1)]++;
        h596_265._red[*(ptr+2)]++;

        // Extract pixel at location [217, 137]
        ptr = GET_PIXEL_PTR_AT(img, 217, 137);

        // Update each histogram for that pixel
        h217_137._blue[*ptr]++;
        h217_137._green[*(ptr+1)]++;
        h217_137._red[*(ptr+1)]++;

        cvReleaseImage(&img);
    }

    // Output histograms in CSV format
    writeHistogram("output/histo10x10.csv", &h10_10);
    writeHistogram("output/histo596x265.csv", &h596_265);
    writeHistogram("output/histo217x137.csv", &h217_137);

    return 0;
}
