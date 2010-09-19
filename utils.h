#include "cv.h"

#include <stdio.h>

void printFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    int row, col;
    for(row = 0; row < frame->height; row++)
    {
        for(col = 0; col < frame->width; col++)
        {
            float* f = (float*)GET_PTR_AT(frame, row, col);

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
            uchar* f = (uchar*)GET_PTR_AT(frame, row, col);

            fprintf(fp, "%d,", *(f+channel));
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp);
}
