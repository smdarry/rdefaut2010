#ifndef _UTILS_H_
#define _UTILS_H_

#include "cv.h"
#include <stdio.h>

void printFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    int row, col, step = frame->widthStep;
    for(row = 0; row < frame->height; row++)
    {
        for(col = 0; col < frame->width; col++)
        {
            float f = ((float*)(frame->imageData + step*row))[col*3];
            fprintf(fp, "%.2f,", f);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void printByteFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    int row, col, step = frame->widthStep;
    for(row = 0; row < frame->height; row++)
    {
        for(col = 0; col < frame->width; col++)
        {
            uchar c = ((uchar*)(frame->imageData + step*row))[col*3];
            fprintf(fp, "%d,", c);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

#endif
