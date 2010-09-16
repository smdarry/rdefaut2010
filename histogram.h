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
