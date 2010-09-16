#include "cv.h"
#include "highgui.h"
#include "histogram.h"
#include <stdio.h>

#define GET_PIXEL_PTR_AT(img, row, col) img->imageData+row*3+col
#define IMAGE_COUNT 795

int main( int argc, char** argv )
{
    // 3-channel histograms for each target pixel 
    Histogram h10_10_r, h10_10_g, h10_10_b;
    Histogram h596_265_r, h596_265_g, h596_265_b;
    Histogram h217_137_r, h217_137_g, h217_137_b;

    initHistogram(&h10_10_r);
    initHistogram(&h10_10_g);
    initHistogram(&h10_10_b);

    initHistogram(&h596_265_r);
    initHistogram(&h596_265_g);
    initHistogram(&h596_265_b);

    initHistogram(&h217_137_r);
    initHistogram(&h217_137_g);
    initHistogram(&h217_137_b);

    // Chronogram for each pixel
    char c10_10_r[IMAGE_COUNT], c10_10_g[IMAGE_COUNT], c10_10_b[IMAGE_COUNT];
    char c596_265_r[IMAGE_COUNT], c596_265_g[IMAGE_COUNT], c596_265_b[IMAGE_COUNT];
    char c217_137_r[IMAGE_COUNT], c217_137_g[IMAGE_COUNT], c217_137_b[IMAGE_COUNT];

    int i, row;
    char filename[256];
    IplImage* img;
    for(i = 0; i < IMAGE_COUNT; i++)
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
        h10_10_b.dist[*ptr]++;
        h10_10_g.dist[*(ptr+1)]++;
        h10_10_r.dist[*(ptr+2)]++;

        // Compile chronogram values
        c10_10_b[i] = *ptr;
        c10_10_g[i] = *(ptr+1);
        c10_10_r[i] = *(ptr+2);

        // Extract pixel at location [596, 265]
        ptr = GET_PIXEL_PTR_AT(img, 596, 265);

        // Update each histogram for that pixel
        h596_265_b.dist[*ptr]++;
        h596_265_g.dist[*(ptr+1)]++;
        h596_265_r.dist[*(ptr+2)]++;

        // Compile chronogram values
        c596_265_b[i] = *ptr;
        c596_265_g[i] = *(ptr+1);
        c596_265_r[i] = *(ptr+2);

        // Extract pixel at location [217, 137]
        ptr = GET_PIXEL_PTR_AT(img, 217, 137);

        // Update each histogram for that pixel
        h217_137_b.dist[*ptr]++;
        h217_137_g.dist[*(ptr+1)]++;
        h217_137_r.dist[*(ptr+2)]++;

        // Compile chronogram values
        c217_137_b[i] = *ptr;
        c217_137_g[i] = *(ptr+1);
        c217_137_r[i] = *(ptr+2);

        cvReleaseImage(&img);
    }

    // Output histograms in CSV format
    writeHistogram(&h10_10_r, "output/histo10x10.csv");
    writeHistogram(&h596_265_r, "output/histo596x265.csv");
    writeHistogram(&h217_137_r, "output/histo217x137.csv");

    int median_10_10 = computeMedian(&h10_10_r);
    printf("Median value for [10,10] pixel: %d\n", median_10_10);

    int median_596_265 = computeMedian(&h596_265_r);
    printf("Median value for [596,265] pixel: %d\n", median_596_265);

    return 0;
}
