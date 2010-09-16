#include <stdio.h>

#define GRAY_LEVELS 256

typedef struct _histogram
{
    int dist[GRAY_LEVELS];
} Histogram;

void initHistogram(Histogram* h)
{
    int i;
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        h->dist[i] = 0;
    }
}

void writeHistogram(Histogram* h, char* filename)
{
    FILE* fp = fopen(filename, "w+");
    if(fp == NULL)
    {
        char buf[256];
        sprintf(buf, "Could not create file '%s'\n", filename);
        fprintf(stderr, buf);

        return;
    }

    int i;
    for(i = 0; i < GRAY_LEVELS-1; i++)
    {
        fprintf(fp, "%d,", (int)h->dist[i]);
    }
    fprintf(fp, "%d\n", (int)h->dist[GRAY_LEVELS-1]);

    fclose(fp);
}

int count(Histogram* h)
{
    int i, N = 0;    
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        N += h->dist[i];
    }
    return N;
}

/**
 * Return the bin index corresponding to the median value.
 */
float computeMedian(Histogram* h)
{
    // Find in which bin the half total count falls in
    int i, A = 0, N_2 = count(h) / 2;
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        A += h->dist[i];
        if(A > N_2)
            return (i - 1);
    }
    return -1;
}

void computeAvgSdv(Histogram* h, float* avg, float* sdv)
{
    int i, N = count(h);

    // Weighed mean
    float a;
    for(i = 0; i < GRAY_LEVELS; i++)
    {
        a += h->dist[i] * i;
    }
    *avg = (a / (float)N);

    // Standard deviation
    if(sdv != NULL)
    {
        float s, u;
        for(i = 0; i < GRAY_LEVELS; i++)
        {
            u = (h->dist[i] - *avg);
            s+= u * u * i;
        }
        *sdv = sqrt(s / (float)N);
    }
}
