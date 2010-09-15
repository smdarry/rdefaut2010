#include "cv.h"
#include "highgui.h"
#include <stdio.h>

#define GET_PIXEL_AT_PTR(img, row, col) img->imageData+row*3+col

int main( int argc, char** argv )
{
    int histoRed[3][255];
    int histoGreen[3][255];
    int histoBlue[3][255];
	
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
        ptr = GET_PIXEL_AT_PTR(img, 10, 10);

        // Update each histogram for that pixel
        histoBlue[0][*ptr]++;
        histoRed[0][*(ptr+1)]++;
        histoGreen[0][*(ptr+2)]++;

        // Extract pixel at location [596, 265]
        ptr = GET_PIXEL_AT_PTR(img, 596, 265);

        // Update each histogram for that pixel
        histoBlue[1][*ptr]++;
        histoRed[1][*(ptr+1)]++;
        histoGreen[1][*(ptr+2)]++;

        // Extract pixel at location [217, 137]
        ptr = GET_PIXEL_AT_PTR(img, 217, 137);

        // Update each histogram for that pixel
        histoBlue[2][*ptr]++;
        histoRed[2][*(ptr+1)]++;
        histoGreen[2][*(ptr+2)]++;

        cvReleaseImage(&img);
    }

    return 0;
}
