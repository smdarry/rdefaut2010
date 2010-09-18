#include "cv.h"
#include "histogram.h"

void testWrite(Histogram* h)
{
    write(h, "testHist.csv");
}

void testComputeMedian(Histogram* h)
{
    float medianBlue = computeMedian(h, 0);
    float medianGreen = computeMedian(h, 1);
    float medianRed = computeMedian(h, 2);

    printf("Median (blue) = %f\n", medianBlue);
    printf("Median (green) = %f\n", medianGreen);
    printf("Median (red) = %f\n", medianRed);
}

int main( int argc, char** argv )
{
    Histogram h1;
    init(&h1);

    ////////////////////
    // Test 1
    //
    char testData[3][9] = { {6,1,4,3,2,4,1,6,9},
                        {13,9,11,11,10,11,12,10,11},
                        {1,0,0,1,2,1,1,2,0} };

    int channel, i;
    for(channel = 0; channel < 3; channel++)
    {
        for(i = 0; i < 9; i++)
        {
            h1.freq[channel][testData[channel][i]]++;
        }
    }

    testWrite(&h1);
    testComputeMedian(&h1);


    /////////////////////
    // Test 2
    //
    Histogram h2;
    init(&h2);

    char testData2[3][8] = { {9,6,12,1,3,1,5,1},
                        {10,5,5,9,9,10,9,5},
                        {0,0,0,0,1,1,1,1} };
    for(channel = 0; channel < 3; channel++)
    {
        for(i = 0; i < 8; i++)
        {
            h2.freq[channel][testData2[channel][i]]++;
        }
    }
    testComputeMedian(&h2);
}
