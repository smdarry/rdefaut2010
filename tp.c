#include <stdio.h>
#include <stdlib.h>

#include "cv.h"
#include "highgui.h"

#include "blob.h"
#include "models.h"
#include "utils.h"

#define IMAGE_COUNT 795

void writeDistanceMatrix(CvMat* mat)
{
    FILE* fp = fopen("distanceMatrix.csv", "w+");
    
    float f;
    int row,col,step = mat->step;
    for(row = 0; row < mat->rows; row++)
    {
        for(col = 0; col < mat->cols; col++)
        {
            f = ((float*)(mat->data.ptr + row*step))[col];
            fprintf(fp, "%.2f,", f);
        }
        f = ((float*)(mat->data.ptr + row*step))[col];
        fprintf(fp, "%.2f\n", f);
    }

    fclose(fp);
}

void playLoop(IplImage* frameBuffer[], int frameCount)
{
    Blob* previousBlobs = NULL;
    int previousBlobCount = 0;

    /////////////////////////////////////////// 
    // Etape 1: construction du modele de fond
    MedianModel medianModel;

    // Apprentissage du modele
    learnMedianModel(&medianModel, frameBuffer, frameCount, 0.95);
    
    char filename[255];
    int i, pb, b;
    IplImage* frame = NULL, *segFrame = NULL;
    for(i = 0; i < 2/*frameCount*/; i++)
    {
        frame = frameBuffer[i];
        
        ////////////////////////////////////////////
        // Etape 2: segmentation utilisant un modele
        segFrame = segmentMedianStdDev(frame, 2.0, &medianModel);
        
        opening(segFrame, segFrame, 3);
        closing(segFrame, segFrame, 3);
        

        ///////////////////////////////////////////////////////////
        // Etape 3: extraction des blobs et de ses caracteristiques
        Blob* blobs;
        int blobCount = extractBlobs(segFrame, frame, &blobs);
    
        drawBoundingRects(segFrame, blobs, blobCount);
        sprintf(filename, "bbox_%04d.jpg", i);
        cvSaveImage(filename, segFrame);
    
        if(previousBlobs != NULL)
        {
            printf("Previous count: %d\n", previousBlobCount);
            printf("Current count: %d\n", blobCount);

            // Matrice des combinaisons de recouvrements spatiaux
            CvMat* mSpatial = cvCreateMat(previousBlobCount, 
                                          blobCount, CV_32FC1);
            float coverage;
            int step = mSpatial->step;
            for(pb = 0; pb < previousBlobCount; pb++)
            {
                for(b = 0; b < blobCount; b++)
                {
                    coverage = percentOverlap(&previousBlobs[pb], &blobs[b]);
                    ((float*)(mSpatial->data.ptr + pb*step))[b] = coverage;
                }
            }
            writeDistanceMatrix(mSpatial);
        }

        previousBlobCount = blobCount;
        previousBlobs = blobs;


        //////////////////////////////////////////////////////////
        // Etape 4: association temporelle avec le frame precedent
        
    }
}

int main(int argc, char *argv[])
{    
    // Prend un echantillon d'images
    int frameCount = 296 - 189 + 1;
    IplImage* frameBuffer[frameCount];

    //selectFrames("../View_008", frameBuffer, frameCount, 1);
    pickFrames("../View_008", frameBuffer, 189, 296);

    // On fait rouler la machine
    playLoop(frameBuffer, frameCount);
    
    return 0;
}
