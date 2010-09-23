#ifndef _UTILS_H_
#define _UTILS_H_

#include "cv.h"
#include <stdio.h>

void printFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    // Print index row
    int i;
    for(i = 0; i < frame->width-1; i++)
        fprintf(fp, "%d,", i);
    fprintf(fp, "%d\n", frame->width-1);

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

void printPatch(IplImage* frame, int channel, char* filename, int beginX, int beginY, int endX, int endY)
{
    uchar pixels[endX - beginX];

    FILE* fp = fopen(filename, "w+");

    int i = 0, row, col, step = frame->widthStep;
    for(row = beginY; row <= endY; row++)
    {
        for(col = beginX; col <= endX; col++)
        {
            pixels[i] = ((uchar*)(frame->imageData + step*row))[col*3];
            fprintf(fp, "%d,", pixels[i]);
            i++;
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void printPatchF(IplImage* frame, int channel, char* filename, int beginX, int beginY, int endX, int endY)
{
    FILE* fp = fopen(filename, "w+");

    int i = 0, row, col, step = frame->widthStep;
    for(row = beginY; row <= endY; row++)
    {
        for(col = beginX; col <= endX; col++)
        {
            float f = ((float*)(frame->imageData + step*row))[col*3];
            fprintf(fp, "%.2f,", f);
            i++;
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

#endif
