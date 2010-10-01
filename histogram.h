#include <stdio.h>
#include "utils.h"

typedef struct _histogram
{
    int channels;
    int bins;
    float binWidth;
    int** freq;
} Histogram;

void initHistogram(Histogram* h, int bins, int channels)
{
    h->channels = channels;
    h->bins = bins;
    h->binWidth = 256.0 / bins;
    h->freq = (int**)malloc(bins*sizeof(int*));

    int i;
    for(i = 0; i < bins; i++)
        h->freq[i] = calloc(channels, sizeof(int));
}

void releaseHistogram(Histogram* h)
{
    int i;
    for(i = 0; i < h->bins; i++)
        free(h->freq[i]);
    free(h->freq);
}

void updateHistogram(Histogram* h, IplImage* frame, int x, int y)
{
    int step = frame->widthStep;

    // Obtient les composantes du pixel a [x, y]
    uchar blue = ((uchar*)(frame->imageData + step*y))[x*3];
    uchar green = ((uchar*)(frame->imageData + step*y))[x*3+1];
    uchar red = ((uchar*)(frame->imageData + step*y))[x*3+2];

    int binBlue = blue / h->binWidth;
    int binGreen = green / h->binWidth;
    int binRed = red / h->binWidth;

    // Mise a jour de chaque plan pour ce pixel
    h->freq[binBlue][0]++;
    h->freq[binGreen][1]++;
    h->freq[binRed][2]++;
}

void writeHistogram(Histogram* h, char* filename)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        fprintf(stderr, "Erreur de creation du fichier '%s'\n", filename);
        return;
    }

    int channel, i, freq;
    for(channel = 0; channel < h->channels; channel++)
    {
        for(i = 0; i < h->bins-1; i++)
        {
            freq = h->freq[i][channel];
            fprintf(fp, "%d,", freq);
        }
        freq = h->freq[i][channel];
        fprintf(fp, "%d\n", freq);
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
