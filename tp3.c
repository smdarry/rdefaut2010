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
    static int nextLabel = 1;
    return nextLabel++;
}

void association(Blob* blobs, Blob* pBlobs, DistMetrics* m, int assocMatrix[])
{
    float maxCoverage = 0.0, coverage, absDiff, absDiffMin = 1.0;
    float score, maxScore = 0.0;
    int maxScoreCol = -1;

    // Ceci est necessaire pour eviter les associations multiples
    int i;
    float maxScores[m->mSpatial->cols];
    for(i = 0; i < m->mSpatial->cols; i++)
        maxScores[i] = 0.0;

    // Initialisation de la matrice d'association
    // un -1 signifie aucune association faite
    for(i = 0; i < m->mSpatial->rows; i++)
        assocMatrix[i] = -1;

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

            score = coverage * (1.0 - (absDiff / 2.0));
            if(score > maxScore)
            {
                maxScore = score;
                maxScoreCol = pb;
            }

            printf("[%d,%d] <--> [%d,%d]: %.2f\n", blobs[b].box.x, blobs[b].box.y, pBlobs[pb].box.x, pBlobs[pb].box.y, score);
        }

        if(maxScore > maxScores[maxScoreCol])
        {
            maxScores[maxScoreCol] = maxScore;
            assocMatrix[b] = maxScoreCol;
        }
        maxScore = 0.0;
    }
}

void playLoop(char* dir, char* filePattern, int firstFrame, int lastFrame)
{
    // Modele de segmentation
    MedianModel medianModel;
    medianModel.isInitialized = 0;

    // Objets en mouvement
    Blob *pBlobs = NULL, *blobs = NULL;
    Blob *outdatedBlobs = NULL;
    int pBlobCount = 0;

    CvMemStorage* storage = cvCreateMemStorage(0);

    char filename[255];
    int i, pb, b;
    IplImage* frame = NULL, *segFrame = NULL;
    for(i = firstFrame; i < lastFrame; i++)
    {
        sprintf(filename, filePattern, dir, i);

        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(frame == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        //////////////////////////////////////////////////
        // Etape 1: mise a jour du modele median adaptatif

        updateMedianModel(&medianModel, frame);
        

        ////////////////////////////////////////////
        // Etape 2: segmentation utilisant le modele
         
        segFrame = segmentMedian(frame, 15.0, &medianModel);
        
        opening(segFrame, segFrame, 3);
        closing(segFrame, segFrame, 3);
        

        ///////////////////////////////////////////////////////////
        // Etape 3: extraction des blobs et de leur caracteristiques
        
        DistMetrics m;

        // Extraction des blobs
        int blobCount = extractBlobs(segFrame, frame, &blobs, storage);

        // Calcul des histogrammes
        for(b = 0; b < blobCount; b++)
        {
            buildHistograms(&blobs[b], frame);

            normalizeHistogram(&blobs[b].h5);
            normalizeHistogram(&blobs[b].h10);
            normalizeHistogram(&blobs[b].h15);
        }

        if(pBlobs != NULL && blobCount > 0 && pBlobCount > 0)
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

            // Transfer des identites (etiquettes) aux nouveaux blobs
            for(b = 0; b < blobCount; b++)
            {
                int index = assocMatrix[b];
                if(index != -1)
                    blobs[b].label = pBlobs[index].label;
                else
                    blobs[b].label = generateLabel();
            }

            // Evaluate velocity for each blob
            for(b = 0; b < blobCount; b++)
            {
                int index = assocMatrix[b];
                velocity(&pBlobs[index], &blobs[b]);
            }
        }
        else
        {
            // Attribution d'une premiere etiquette a chaque blob
            for(b = 0; b < blobCount; b++)
                blobs[b].label = generateLabel();
        }

        // Images binaires
        drawBoundingRects(segFrame, blobs, blobCount);
        drawLabels(segFrame, blobs, blobCount);
        drawVelocityVectors(segFrame, blobs, blobCount);
        sprintf(filename, "bbox_%04d.jpg", i);
        cvSaveImage(filename, segFrame);

        // Image originales
        drawBoundingRects(frame, blobs, blobCount);
        drawLabels(frame, blobs, blobCount);
        drawVelocityVectors(frame, blobs, blobCount);
        sprintf(filename, "suivi_%04d.jpg", i);
        cvSaveImage(filename, frame);

        outdatedBlobs = pBlobs;
        pBlobCount = blobCount;
        pBlobs = blobs;

        cvReleaseImage(&frame);
        cvReleaseImage(&segFrame);

        if(outdatedBlobs != NULL)
            releaseBlobs(outdatedBlobs);
    }
    cvReleaseMemStorage(&storage);
}

int main(int argc, char *argv[])
{    
    // On fait rouler la machine
    playLoop("../View_006", "%s/128691%06d7.jpg", 87968, 159522);
    
    return 0;
}
