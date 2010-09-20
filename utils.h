#ifndef _UTILS_H_
#define _UTILS_H_

#include "cv.h"

#include <stdio.h>

#define GET_PTR_AT(img, x, y) img->imageData+img->widthStep*y+x*3

void printFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    int row, col;
    for(row = 0; row < frame->height; row++)
    {
        for(col = 0; col < frame->width; col++)
        {
            float* f = (float*)(frame->imageData + frame->width*row*3 + col*3); 

            fprintf(fp, "%.2f,", *(f+channel));
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void printByteFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    int row, col;
    for(row = 0; row < frame->height; row++)
    {
        for(col = 0; col < frame->width; col++)
        {
            uchar* f = (uchar*)GET_PTR_AT(frame, col, row);

            fprintf(fp, "%d,", *(f+channel));
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

#endif
