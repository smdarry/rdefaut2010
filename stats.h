#include "cv.h"
#include <math.h>
#include <stdlib.h>

int cmpPixel(const void *p1, const void *p2)
{
    return *((const uchar*)p1) - *((const uchar*)p2);
}

float computeMedian(uchar pixelArray[], int size)
{
    float median;
    int size_2 = size / 2;

    qsort(pixelArray, size, sizeof(uchar), cmpPixel);
    if(size % 2 == 0)
        median = (pixelArray[size_2-1] + pixelArray[size_2]) / 2.0;
    else
        median = pixelArray[size_2];
    
    return median;
}

float computePercentile(uchar pixels[], int size, float percentile)
{
    float r = percentile * (size - 1) + 1;
    int rHigh = (int)ceil(r);
    int rLow = (int)floor(r);
    float residue = r - rLow;

    qsort(pixels, size, sizeof(uchar), cmpPixel);
    
    uchar pHigh = pixels[rHigh-1];
    uchar pLow = pixels[rLow-1];

    // Interpole la valeur
    float p = residue * (pHigh - pLow) + pLow;
    
    return p;
}

void computeMeanSdv(uchar pixelArray[], int size, float* mean, float* sdv)
{
    float m = 0.0, s = 0.0;
    
    // Moyenne
    int i;
    for(i = 0; i < size; i++)
    {
        m += pixelArray[i];
    }
    m /= size;

    if(mean != NULL)
        *mean = m;

    // et l'ecart-type...
    if(sdv == NULL) return;

    float sdiff;
    for(i = 0; i < size; i++)
    {
        sdiff = pixelArray[i] - m;
        s += sdiff * sdiff;
    }
    s = sqrt(s / size);

    if(sdv != NULL)
        *sdv = s;
}
