#include <stdio.h>
#include <stdlib.h>

#include "cv.h"
#include "highgui.h"

#include "blob.h"
#include "models.h"
#include "utils.h"

#define IMAGE_COUNT 795

typedef struct _metrics
{
    CvMat* mSpatial;
    CvMat* mHist5;
    CvMat* mHist10;
    CvMat* mHist15;
} DistMetrics;

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

int generateLabel()
{
    static int nextLabel = 0;
    return nextLabel++;
}

void association(Blob* blobs, Blob* pBlobs, DistMetrics* m, int assocMatrix[])
{
    float maxCoverage = 0.0, coverage, absDiff, absDiffMin = 1.0;
    float score, maxScore = 0.0;
    int maxScoreCol = -1;

    uchar* datam = m->mSpatial->data.ptr;
    int stepm = m->mSpatial->step;

    uchar* datad = m->mHist5->data.ptr;
    int stepd = m->mHist5->step;

    int b, pb;
    for(b = 0; b < m->mSpatial->rows; b++)
    {
        for(pb = 0; pb < m->mSpatial->cols; pb++)
        {
            coverage = ((float*)(datam + stepm*b))[pb];
            absDiff = ((float*)(datad + stepd*b))[pb*3];

            printf("Coverage = %.2f\n", coverage);
            printf("Abs diff = %.2f\n", absDiff);

            score = coverage * (2.0 - absDiff);
            if(score > maxScore)
            {
                maxScore = score;
                maxScoreCol = pb;
            }

            printf("Score for (%d,%d) = %.2f\n", b, pb, score);
        }
        assocMatrix[b] = maxScoreCol;
    }
}

/*
        // Cas 1: maxScore == 0 => nouveau blob
        // Cas 2: maxScore > 0 => association
        if(maxScore == 0.0)
        {
            // Assigne un nouveau label
            blobs[b].label = generateLabel();
        }
        else
        {
            // Accepte le label du meilleur match
            blobs[b].label = pBlobs[maxScoreCol].label;
        }

        // cas 3: blob disparait
    }
}
*/

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
        DistMetrics m;

        // Extraction des blobs
        int blobCount = extractBlobs(segFrame, frame, &blobs);
    
        drawBoundingRects(segFrame, blobs, blobCount);
        drawLabels(segFrame, blobs, blobCount);

        sprintf(filename, "bbox_%04d.jpg", i);
        cvSaveImage(filename, segFrame);

        // Calcul des histogrammes pour chaque blob
        for(b = 0; b < blobCount; b++)
        {
            buildHistograms(&blobs[b], frame);

            normalizeHistogram(&blobs[b].h5);
            normalizeHistogram(&blobs[b].h10);
            normalizeHistogram(&blobs[b].h15);
        }
    
        if(pBlobs != NULL)
        {
            // Matrice des combinaisons de recouvrements spatiaux
            m.mSpatial = cvCreateMat(blobCount, pBlobCount, CV_32FC1);

            // Matrices des differences d'histogramme
            m.mHist5 = cvCreateMat(blobCount, pBlobCount, CV_32FC3);
            m.mHist10 = cvCreateMat(blobCount, pBlobCount, CV_32FC3);
            m.mHist15 = cvCreateMat(blobCount, pBlobCount, CV_32FC3);

            float coverage, absDiff;
            Blob *b1, *b2;
            int step = m.mSpatial->step, hstep = m.mHist5->step;
            for(b = 0; b < blobCount; b++)
            {
                for(pb = 0; pb < pBlobCount; pb++)
                {
                    b1 = &blobs[b];
                    b2 = &pBlobs[pb];

                    coverage = percentOverlap(b1, b2);
                    ((float*)(m.mSpatial->data.ptr + b*step))[pb] = coverage;

                    absDiff = absDiffHistograms(&b1->h5, &b2->h5, 0);
                    ((float*)(m.mHist5->data.ptr + b*hstep))[pb*3] = absDiff;

                    absDiff = absDiffHistograms(&b1->h10, &b2->h10, 0);
                    ((float*)(m.mHist10->data.ptr + b*hstep))[pb*3] = absDiff;

                    absDiff = absDiffHistograms(&b1->h15, &b2->h15, 0);
                    ((float*)(m.mHist15->data.ptr + b*hstep))[pb*3] = absDiff;

                    // TODO: Faire les autres canaux
                }
            }


            //////////////////////////////////////////////////////////
            // Etape 4: association temporelle avec le frame precedent
            int assocMatrix[blobCount];
            association(blobs, pBlobs, &m, assocMatrix);

            // Print association matrix
            //for(b = 0; b < blobCount; b++)
            //    printf("%d <--> %d\n", b, assocMatrix[b]);
        }
        else
        {
            // Attribution d'une etiquette a chaque blob
            for(b = 0; b < blobCount; b++)
                blobs[b].label = generateLabel();
        }

        pBlobCount = blobCount;
        pBlobs = blobs;
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
