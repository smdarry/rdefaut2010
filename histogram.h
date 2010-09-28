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

    // Obtient les composantes du pixel a [x, y]
    uchar blue = ((uchar*)(frame->imageData + step*y))[x*3];
    uchar green = ((uchar*)(frame->imageData + step*y))[x*3+1];
    uchar red = ((uchar*)(frame->imageData + step*y))[x*3+2];

    // Mise a jour de chaque plan pour ce pixel
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

    // Obtient pixel en [x, y]
    uchar blue = ((uchar*)(frame->imageData + step*y))[x*3];
    uchar green = ((uchar*)(frame->imageData + step*y))[x*3+1];
    uchar red = ((uchar*)(frame->imageData + step*y))[x*3+2];

    // Mise a jour des intensites au temps 't'
    ((uchar*)c->data.ptr)[t*3] = blue;
    ((uchar*)c->data.ptr)[t*3+1] = green;
    ((uchar*)c->data.ptr)[t*3+2] = red;
}

void writeChronogram(CvMat* c, char* filename)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        fprintf(stderr, "Erreur de creation du fichier '%s'\n", filename);
        return;
    }

    uchar pixel;
    int channel, t;
    for(channel = 0; channel < 3; channel++)
    {
        for(t = 0; t < c->cols-1; t++)
        {
            pixel = ((uchar*)c->data.ptr)[t*3+channel];
            fprintf(fp, "%d,", pixel);
        }
        pixel = ((uchar*)c->data.ptr)[t*3+channel];
        fprintf(fp, "%d\n", pixel);
    }
    fclose(fp);
}

void updateRunningMean(CvMat* c, IplImage* frame, int t, int x, int y, float a)
{
    int stepu = frame->widthStep;

    // Obtient pointeur sur le pixel a [x, y]
    uchar blue = ((uchar*)(frame->imageData + stepu*y))[x*3];
    uchar green = ((uchar*)(frame->imageData + stepu*y))[x*3+1];
    uchar red = ((uchar*)(frame->imageData + stepu*y))[x*3+2];

    if(t == 0)
    {
        // Initialisation au temps 't = 0'
        ((float*)c->data.ptr)[0] = (float)blue;
        ((float*)c->data.ptr)[1] = (float)green;
        ((float*)c->data.ptr)[2] = (float)red;
    }
    else
    {
        // Obtient les moyennes au temps 't-1'
        float meanBlue = ((float*)c->data.ptr)[(t-1)*3];
        float meanGreen = ((float*)c->data.ptr)[(t-1)*3+1];
        float meanRed = ((float*)c->data.ptr)[(t-1)*3+2];

        // Mise a jour de chaque plan au temps 't'
        ((float*)c->data.ptr)[t*3] = a * blue + (1 - a) * meanBlue;
        ((float*)c->data.ptr)[t*3+1] = a * green + (1 - a) * meanGreen;
        ((float*)c->data.ptr)[t*3+2] = a * red + (1 - a) * meanRed;
    }
}

void writeRunningMean(CvMat* c, char* filename)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        fprintf(stderr, "Erreur de creation du fichier '%s'\n", filename);
        return;
    }

    float mean;
    int channel, t;
    for(channel = 0; channel < 3; channel++)
    {
        for(t = 0; t < c->cols-1; t++)
        {
            mean = ((float*)c->data.ptr)[t*3+channel];
            fprintf(fp, "%.2f,", mean);
        }
        mean = ((float*)c->data.ptr)[t*3+channel];
        fprintf(fp, "%.2f\n", mean);
    }
    fclose(fp);
}
