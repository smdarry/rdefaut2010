#include <stdio.h>

#define GRAY_LEVELS 256
#define GET_PTR_AT(img, x, y) img->imageData+img->widthStep*y+x*3

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
    // Pointer on pixel of first channel (blue)
    uchar* ptr;

    // Extract pixel at location [x, y]
    ptr = GET_PTR_AT(frame, x, y);

    // Update each histogram for that pixel
    h->freq[0][*ptr]++;     // Blue
    h->freq[1][*(ptr+1)]++; // Green
    h->freq[2][*(ptr+2)]++; // Red
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
    uchar *pixel, *ptrDst;

    // Obtient pointeur sur le pixel a [x, y]
    pixel = GET_PTR_AT(frame, x, y);

    // Obtient un pointeur sur la destination au temps 't'
    ptrDst = (uchar*)(c->data.ptr + t*3);

    // Mise a jour de chaque plan au temps 't'
    *ptrDst = *pixel;           // Blue
    *(ptrDst+1) = *(pixel+1);   // Green
    *(ptrDst+2) = *(pixel+2);   // Red
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
