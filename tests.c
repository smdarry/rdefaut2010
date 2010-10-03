#include <stdio.h>

#include "blob.h"
#include "histogram.h"
#include "stats.h"
#include "utils.h"

void testComputeMedian()
{
    ////////////////////
    // Test 1
    //
    char testData1[3][9] = { {6,1,4,3,2,4,1,6,9},
                        {13,9,11,11,10,11,12,10,11},
                        {1,0,0,1,2,1,1,2,0} };
    float median;

    printf("\nTEST MEDIAN\n");

    median = computeMedian(testData1[0], 9);
    printf("Expected: 4.0, Actual: %.1f\n", median);

    median = computeMedian(testData1[1], 9);
    printf("Expected: 11.0, Actual: %.1f\n", median);

    median = computeMedian(testData1[2], 9);
    printf("Expected: 1.0, Actual: %.1f\n", median);


    /////////////////////
    // Test 2
    //
    char testData2[3][8] = { {9,6,12,1,3,1,5,1},
                        {10,5,5,9,9,10,9,5},
                        {0,0,0,0,1,1,1,1} };
    
    median = computeMedian(testData2[0], 8);
    printf("Expected: 4.0, Actual: %.1f\n", median);

    median = computeMedian(testData2[1], 8);
    printf("Expected: 9.0, Actual: %.1f\n", median);

    median = computeMedian(testData2[2], 8);
    printf("Expected: 0.5, Actual: %.1f\n", median);
}

void testComputePercentile()
{
    ////////////////////
    // Test 1
    //
    char testData1[3][9] = { {5,1,4,3,2,4,1,6,9},
                        {13,9,11,11,10,11,12,10,11},
                        {1,0,0,1,2,1,1,2,0} };

    printf("\nTEST PERCENTILE\n");

    float percentile = computePercentile(testData1[0], 9, 0.95);
    printf("Expected: 7.80, Actual: %.2f\n", percentile);

    percentile = computePercentile(testData1[0], 9, 0.05);
    printf("Expected: 1.00, Actual: %.2f\n", percentile);

    percentile = computePercentile(testData1[0], 9, 0.7);
    printf("Expected: 4.60, Actual: %.2f\n", percentile);

    percentile = computePercentile(testData1[0], 9, 1.0);
    printf("Expected: 9.00, Actual: %.2f\n", percentile);

    percentile = computePercentile(testData1[0], 9, 0.0);
    printf("Expected: 1.00, Actual: %.2f\n", percentile);

    percentile = computePercentile(testData1[0], 9, 0.5);
    printf("Expected: 4.00, Actual: %.2f\n", percentile);
}

void testComputeMeanSdv()
{
    /////////////////////
    // Test 1 
    //
    char testData1[3][9] = { {6,1,4,3,2,4,1,6,9},
                        {13,9,11,11,10,11,12,10,11},
                        {1,0,0,1,2,1,1,2,0} };
    float mean, sdv;

    printf("\nTEST MEAN and STANDARD DEV.\n");

    computeMeanSdv(testData1[0], 9, &mean, &sdv);
    printf("Expected: 4.0, Actual: %.1f\n", mean);
    printf("Expected: 2.49, Actual: %.2f\n", sdv);

    computeMeanSdv(testData1[1], 9, &mean, &sdv);
    printf("Expected: 10.89, Actual: %.2f\n", mean);
    printf("Expected: 1.09, Actual: %.2f\n", sdv);

    computeMeanSdv(testData1[2], 9, &mean, &sdv);
    printf("Expected: 0.89, Actual: %.2f\n", mean);
    printf("Expected: 0.74, Actual: %.2f\n", sdv);
}

void testCvThreshold()
{
    IplImage* image = cvCreateImage(cvSize(720, 576), IPL_DEPTH_32F, 3);
    cvZero(image);

    int step = image->widthStep;

    int row, col;
    for(row = 0; row < 576; row++)
    {
        for(col = 0; col < 720; col++)
        {
            ((float*)(image->imageData + step*row))[col*3] = 10.0;
            ((float*)(image->imageData + step*row))[col*3+1] = 11.0;
            ((float*)(image->imageData + step*row))[col*3+2] = 12.0;
        }
    }

    for(row = 0; row < 576; row++)
    {
        for(col = 0; col < 720; col++)
        {
            float p = ((float*)(image->imageData + step*row))[col*3+1];
            printf("[%d,%d] : %.2f\n", col, row, p);
        }
    }
    cvReleaseImage(&image);
}

void testOverlappingBlobs()
{
    printf("\nTEST BLOB OVERLAPPING\n");
    
    Blob b1, b2;
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 3; b1.box.height = 3;
    b2.box.x = 2; b2.box.y = 3;
    b2.box.width = 3; b2.box.height = 2;
    
    int overlap = areOverlapping(&b1, &b2);
    printf("Expected: 1, Actual: %d\n", overlap);
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 3; b1.box.height = 3;
    b2.box.x = 3; b2.box.y = 1;
    b2.box.width = 1; b2.box.height = 3;
    
    overlap = areOverlapping(&b1, &b2);
    printf("Expected: 1, Actual: %d\n", overlap);
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 3; b1.box.height = 3;
    b2.box.x = 4; b2.box.y = 1;
    b2.box.width = 1; b2.box.height = 3;
    
    overlap = areOverlapping(&b1, &b2);
    printf("Expected: 0, Actual: %d\n", overlap);
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 3; b1.box.height = 3;
    b2.box.x = 2; b2.box.y = 5;
    b2.box.width = 1; b2.box.height = 3;
    
    overlap = areOverlapping(&b1, &b2);
    printf("Expected: 0, Actual: %d\n", overlap);
}

void testOverlappingBlobsArea()
{
    printf("\nTEST BLOB OVERLAPPING AREA\n");
    
    Blob b1, b2;
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 3; b1.box.height = 3;
    b2.box.x = 2; b2.box.y = 3;
    b2.box.width = 3; b2.box.height = 2;
    
    float area = percentOverlap(&b1, &b2);
    printf("Expected: 0.22, Actual: %.2f\n", area);
    
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 3; b1.box.height = 3;
    b2.box.x = 2; b2.box.y = 5;
    b2.box.width = 1; b2.box.height = 3;
    
    area = percentOverlap(&b1, &b2);
    printf("Expected: 0.00, Actual: %.2f\n", area);
    
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 3; b1.box.height = 3;
    b2.box.x = 2; b2.box.y = 5;
    b2.box.width = 1; b2.box.height = 3;
    
    area = percentOverlap(&b1, &b2);
    printf("Expected: 0.00, Actual: %.2f\n", area);
    
    
    b1.box.x = 1; b1.box.y = 1;
    b1.box.width = 4; b1.box.height = 2;
    b2.box.x = 2; b2.box.y = 2;
    b2.box.width = 2; b2.box.height = 2;
    
    area = percentOverlap(&b1, &b2);
    printf("Expected: 0.25, Actual: %.2f\n", area);
    
    
    b1.box.x = 0; b1.box.y = 0;
    b1.box.width = 2; b1.box.height = 3;
    b2.box.x = 2; b2.box.y = 0;
    b2.box.width = 2; b2.box.height = 3;
    
    area = percentOverlap(&b1, &b2);
    printf("Expected: 0.00, Actual: %.2f\n", area);
    
    
    b1.box.x = 0; b1.box.y = 0;
    b1.box.width = 2; b1.box.height = 3;
    b2.box.x = 1; b2.box.y = 0;
    b2.box.width = 2; b2.box.height = 3;
    
    area = percentOverlap(&b1, &b2);
    printf("Expected: 0.50, Actual: %.2f\n", area);
    
    
    b1.box.x = 0; b1.box.y = 0;
    b1.box.width = 2; b1.box.height = 3;
    b2.box.x = 0; b2.box.y = 0;
    b2.box.width = 2; b2.box.height = 3;
    
    area = percentOverlap(&b1, &b2);
    printf("Expected: 1.00, Actual: %.2f\n", area);
}

void testAbsDiffHistograms()
{
    printf("\nTEST HISTOGRAM DIFFERENCE\n");

    // Construction d'histogrammes de test
    int channel = 0;
    Histogram h1;
    initHistogram(&h1, 5, 3);
    h1.freq[0][channel] = 12;   // 0.11
    h1.freq[1][channel] = 15;   // 0.14
    h1.freq[2][channel] = 20;   // 0.18
    h1.freq[3][channel] = 45;   // 0.41
    h1.freq[4][channel] = 19;   // 0.17
    
    Histogram h2;
    initHistogram(&h2, 5, 3);
    h2.freq[0][channel] = 3;    // 0.02
    h2.freq[1][channel] = 17;   // 0.14
    h2.freq[2][channel] = 19;   // 0.15
    h2.freq[3][channel] = 70;   // 0.56
    h2.freq[4][channel] = 16;   // 0.13

    float sum = absDiffHistograms(&h1, &h2, channel);
    printf("Expected: 40.00, Actual: %.2f\n", sum);

    // Avec histogrammes normalises
    normalizeHistogram(&h1);
    normalizeHistogram(&h2);

    sum = absDiffHistograms(&h1, &h2, channel);
    printf("Expected: 0.31, Actual: %.2f\n", sum);


    // Test 2: histogrammes identiques
    //
    h1.freq[0][channel] = 12;   // 0.11
    h1.freq[1][channel] = 15;   // 0.14
    h1.freq[2][channel] = 20;   // 0.18
    h1.freq[3][channel] = 45;   // 0.41
    h1.freq[4][channel] = 19;   // 0.17

    h2.freq[0][channel] = 12;   // 0.11
    h2.freq[1][channel] = 15;   // 0.14
    h2.freq[2][channel] = 20;   // 0.18
    h2.freq[3][channel] = 45;   // 0.41
    h2.freq[4][channel] = 19;   // 0.17

    normalizeHistogram(&h1);
    normalizeHistogram(&h2);

    sum = absDiffHistograms(&h1, &h2, channel);
    printf("Expected: 0.00, Actual: %.2f\n", sum);


    // Test 3: histogrammes disjoints completement
    //
    h1.freq[0][channel] = 0;   // 0.00
    h1.freq[1][channel] = 0;   // 0.00
    h1.freq[2][channel] = 0;   // 0.00
    h1.freq[3][channel] = 45;  // 0.70
    h1.freq[4][channel] = 19;  // 0.30

    h2.freq[0][channel] = 23;  // 0.56
    h2.freq[1][channel] = 15;  // 0.37
    h2.freq[2][channel] = 3;   // 0.07
    h2.freq[3][channel] = 0;   // 0.00
    h2.freq[4][channel] = 0;   // 0.00

    normalizeHistogram(&h1);
    normalizeHistogram(&h2);

    sum = absDiffHistograms(&h1, &h2, channel);
    printf("Expected: 2.00, Actual: %.2f\n", sum);
}

void testNormalizeHistogram()
{
    printf("\nTEST NORMALIZE HISTOGRAM\n");

    Histogram h1;
    initHistogram(&h1, 5, 3);
    h1.freq[0][0] = 12;
    h1.freq[1][0] = 15;
    h1.freq[2][0] = 20;
    h1.freq[3][0] = 45;
    h1.freq[4][0] = 19;

    h1.freq[0][1] = 5;
    h1.freq[1][1] = 7;
    h1.freq[2][1] = 16;
    h1.freq[3][1] = 6;
    h1.freq[4][1] = 4;

    normalizeHistogram(&h1);
    printf("Expected: 0.11, Actual: %.2f\n", h1.freq[0][0]);
    printf("Expected: 0.14, Actual: %.2f\n", h1.freq[1][0]);
    printf("Expected: 0.18, Actual: %.2f\n", h1.freq[2][0]);
    printf("Expected: 0.41, Actual: %.2f\n", h1.freq[3][0]);
    printf("Expected: 0.17, Actual: %.2f\n", h1.freq[4][0]);

    printf("Expected: 0.13, Actual: %.2f\n", h1.freq[0][1]);
    printf("Expected: 0.18, Actual: %.2f\n", h1.freq[1][1]);
    printf("Expected: 0.42, Actual: %.2f\n", h1.freq[2][1]);
    printf("Expected: 0.16, Actual: %.2f\n", h1.freq[3][1]);
    printf("Expected: 0.11, Actual: %.2f\n", h1.freq[4][1]);
}


int main( int argc, char** argv )
{
    testComputeMedian();

    testComputePercentile();
    
    testComputeMeanSdv();

    testOverlappingBlobs();
    
    testOverlappingBlobsArea();
    
    testAbsDiffHistograms();

    testNormalizeHistogram();
    
    return 0;
}
