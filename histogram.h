#include <stdio.h>
#include "utils.h"

#define GRAY_LEVELS 256

typedef struct _histogram
{
    int freq[3][GRAY_LEVELS];
} Histogram;

void initHistogram(Histogram* h)
{
    int channel, i;
    for(channel = 0; channel < 3; channel++)
    {
        for(i = 0; i < GRAY_LEVELS; i++)
        {
            h->freq[channel][i] = 0;
        }
    }
}

void updateHistogram(Histogram* h, IplImage* frame, int x, int y)
{
    int step = frame->widthStep;

    // Obtient pointeur sur le pixel a [x, y]
    uchar blue = ((uchar*)(frame->imageData + step*y))[x*3];
    uchar green = ((uchar*)(frame->imageData + step*y))[x*3+1];
    uchar red = ((uchar*)(frame->imageData + step*y))[x*3+2];

    // Mise a jour de chaque histogramme pour ce pixel
    h->freq[0][blue]++;
    h->freq[1][green]++;
    h->freq[2][red]++;
}

void writeHistogram(Histogram* h, char* filename)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        fprintf(stderr, "Erreur de creation du fichier '%s'\n", filename);
        return;
    }

    int channel, i;
    for(channel = 0; channel < 3; channel++)
    {
        for(i = 0; i < GRAY_LEVELS-1; i++)
        {
            fprintf(fp, "%d,", (int)h->freq[channel][i]);
        }
        fprintf(fp, "%d\n", (int)h->freq[channel][GRAY_LEVELS-1]);
    }
    fclose(fp);
}

void updateChronogram(CvMat* c, IplImage* frame, int t, int x, int y)
{
    int step = frame->widthStep;

    // Obtient pointeur sur le pixel a [x, y]
    uchar blue = ((uchar*)(frame->imageData + step*y))[x*3];
    uchar green = ((uchar*)(frame->imageData + step*y))[x*3+1];
    uchar red = ((uchar*)(frame->imageData + step*y))[x*3+2];

    // Obtient un pointeur sur la destination au temps 't'
    uchar* ptrDst = (uchar*)(c->data.ptr + t*3);

    // Mise a jour de chaque plan au temps 't'
    *ptrDst = blue;
    *(ptrDst+1) = green;
    *(ptrDst+2) = red;
}

void writeChronogram(CvMat* c, char* filename)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        fprintf(stderr, "Erreur de creation du fichier '%s'\n", filename);
        return;
    }

    uchar* ptr;
    int channel, t;
    for(channel = 0; channel < 3; channel++)
    {
        for(t = 0; t < c->cols-1; t++)
        {
            ptr = (uchar*)(c->data.ptr + t*3);
            fprintf(fp, "%d,", *(ptr+channel));
        }
        fprintf(fp, "%d\n", *(ptr+(c->cols-1)+channel));
    }
    fclose(fp);
}
