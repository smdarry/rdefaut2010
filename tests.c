#include <stdio.h>
#include "stats.h"

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

int main( int argc, char** argv )
{
    testComputeMedian();
    
    testComputeMeanSdv();
}
