#include <stdio.h>
#include <stdlib.h>

#include "cv.h"
#include "highgui.h"

#include "blob.h"
#include "models.h"
#include "tracking.h"
#include "utils.h"

#define IMAGE_COUNT 795

int generateLabel()
{
    static int nextLabel = 1;
    return nextLabel++;
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
    FILE* trackData = fopen("trackData.csv", "w+");

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
        

        ////////////////////////////////////////////////////////////
        // Etape 3: extraction des blobs et de leur caracteristiques
        
        // Extraction des blobs
        int blobCount = extractBlobs(segFrame, frame, &blobs, storage);
        blobCount = mergeBlobs(&blobs, blobCount, 0);

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
            /////////////////////////////////////////////////
            // Etape 4: fusion temporelle des blobs -> mise a jour des tracks
            fusion(blobs, blobCount, pBlobs, pBlobCount);

            int b;
            for(b = 0; b < blobCount; b++)
            {
                if(blobs[b].label == 2)
                {
                    printf("A/R: %.2f\n", blobs[b].aspectRatio);
                    fprintf(trackData, "%.2f,%.2f\n", blobs[b].aspectRatio,
                                            blobs[b].speed);
                    fflush(trackData);
                }
            }
    
            //////////////////////////
            // Etape 5: classification
            classify(blobs, blobCount);
        }
        else
        {
            // Attribution d'une premiere etiquette a chaque blob
            for(b = 0; b < blobCount; b++)
                blobs[b].label = generateLabel();

            // Initialisation du vecteur de vitesse
            for(b = 0; b < blobCount; b++)
                velocity(NULL, &blobs[b]);
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
    fclose(trackData);
}

int main(int argc, char *argv[])
{
    // On fait rouler la machine
    playLoop("../View_006", "%s/128691%06d7.jpg", 87968, 159522);
    //playLoop("../View_006", "%s/128691%06d7.jpg", 88020, 88096);
    
    return 0;
}
