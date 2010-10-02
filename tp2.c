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
    Blob* pBlobs = NULL;
    int pBlobCount = 0;

    /////////////////////////////////////////// 
    // Etape 1: construction du modele de fond
    MedianModel medianModel;

    // Apprentissage du modele
    learnMedianModel(&medianModel, frameBuffer, frameCount, 0.95);
    
    char filename[255];
    int i, pb, b;
    IplImage* frame = NULL, *segFrame = NULL;
    for(i = 0; i < frameCount; i++)
    {
        frame = frameBuffer[i];
        
        ////////////////////////////////////////////
        // Etape 2: segmentation utilisant un modele
         
        segFrame = segmentMedianStdDev(frame, 2.0, &medianModel);
        
        opening(segFrame, segFrame, 3);
        closing(segFrame, segFrame, 3);
        

        ///////////////////////////////////////////////////////////
        // Etape 3: extraction des blobs et de leur caracteristiques
        
        Blob* blobs;

        // Extraction des blobs
        int blobCount = extractBlobs(segFrame, frame, &blobs);
    
        drawBoundingRects(segFrame, blobs, blobCount);
        sprintf(filename, "bbox_%04d.jpg", i);
        cvSaveImage(filename, segFrame);

        // Calcul des histogrammes pour chaque blob
        for(b = 0; b < blobCount; b++)
        {
            buildHistograms(&blobs[b], frame);
        }
    
        if(pBlobs != NULL)
        {
            printf("Previous count: %d\n", pBlobCount);
            printf("Current count: %d\n", blobCount);

            // Matrice des combinaisons de recouvrements spatiaux
            CvMat* mSpatial = cvCreateMat(pBlobCount, blobCount, CV_32FC1);

            // Matrices des differences d'histogramme
            CvMat* mHist5 = cvCreateMat(pBlobCount, blobCount, CV_32FC3);
            CvMat* mHist10 = cvCreateMat(pBlobCount, blobCount, CV_32FC3);
            CvMat* mHist15 = cvCreateMat(pBlobCount, blobCount, CV_32FC3);

            float coverage, absDiff;
            Blob *b1, *b2;
            int step = mSpatial->step, hstep = mHist5->step;
            for(pb = 0; pb < pBlobCount; pb++)
            {
                for(b = 0; b < blobCount; b++)
                {
                    b1 = &pBlobs[pb];
                    b2 = &blobs[b];

                    coverage = percentOverlap(b1, b2);
                    ((float*)(mSpatial->data.ptr + pb*step))[b] = coverage;

                    absDiff = absDiffHistograms(&b1->h5, &b2->h5, 0);
                    ((float*)(mHist5->data.ptr + pb*hstep))[b*3] = absDiff;

                    absDiff = absDiffHistograms(&b1->h10, &b2->h10, 0);
                    ((float*)(mHist10->data.ptr + pb*hstep))[b*3] = absDiff;

                    absDiff = absDiffHistograms(&b1->h15, &b2->h15, 0);
                    ((float*)(mHist15->data.ptr + pb*hstep))[b*3] = absDiff;

                    // TODO: Faire les autres canaux
                }
            }
            writeDistanceMatrix(mSpatial);
        }

        pBlobCount = blobCount;
        pBlobs = blobs;


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
