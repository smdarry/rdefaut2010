#include <stdio.h>

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

Histogram** allocHistogramMatrix(int width, int height)
{
    Histogram** h = malloc(height * sizeof(Histogram*));
    int row;
    for(row = 0; row < height; row++)
        h[row] = malloc(width * sizeof(Histogram));

    return h;
}

void freeHistogramMatrix(Histogram** h, int width, int height)
{
    int row;
    for(row = 0; row < height; row++)
        free(h[row]);
    free(h);
}

void writeHistogram(Histogram* h, char* filename)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        fprintf(stderr, "Could not create file '%s'\n", filename);
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

int count(Histogram* h, int channel)
{
    int i, N = 0;    
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        if(channel == 0)
            N += h->freq[channel][i];
    }
    return N;
}

/**
 * Return the bin index corresponding to the median value.
 */
float calculeMedianne(Histogram* h, int channel)
{
    // Find in which bin the half total count falls in
    int i, A = 0, N_2 = count(h, channel) / 2;
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        if(channel == 0)
            A += h->freq[channel][i];

        if(A > N_2)
            return (i - 1);
    }
    return -1;
}

void calculeMoyEcartType(Histogram* h, float* avg, float* sdv, int channel)
{
    int i, N = count(h, channel);

    // Weighed mean
    float a;
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        if(channel == 0)
            a += h->freq[channel][i] * i;
    }
    *avg = (a / (float)N);

    // Standard deviation
    if(sdv != NULL)
    {
        float s, u;
        for(i = 0; i < GRAY_LEVELS; i++)
        {
            u = (h->freq[channel][i] - *avg);
            s+= u * u * i;
        }
        *sdv = sqrt(s / (float)N);
    }
}
