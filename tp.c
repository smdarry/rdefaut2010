#include <stdio.h>
#include <stdlib.h>

#include "cv.h"
#include "highgui.h"

#include "blob.h"
#include "models.h"
#include "utils.h"

#define IMAGE_COUNT 795

void playLoop(IplImage* frameBuffer[], int frameCount)
{
     /////////////////////////////////////////// 
     // Etape 1: construction du modele de fond
     MedianModel medianModel;

     // Apprentissage du modele
     learnMedianModel(&medianModel, frameBuffer, frameCount, 0.95);

     int i;
     IplImage* frame = NULL, *segFrame = NULL;
     for(i = 0; i < 1/*frameCount*/; i++)
     {
        frame = frameBuffer[i];
        
        ////////////////////////////////////////////
        // Etape 2: segmentation utilisant un modele
        segFrame = segmentMedian(frame, 20, &medianModel);
        
        opening(segFrame, segFrame, 3);
        closing(segFrame, segFrame, 3);
        
        cvSaveImage("medianImage.jpg", segFrame, NULL);

        ///////////////////////////////////////////////////////////
        // Etape 3: extraction des blobs et de ses caracteristiques
        Blob* blobs;
        int blobCount = extractBlobs(segFrame, frame, blobs);
    
        //////////////////////////////////////////////////////////
        // Etape 4: association temporelle avec le frame precedent
    }
}

int main(int argc, char *argv[])
{    
    // Prend un echantillon d'images
    int frameCount = IMAGE_COUNT;
    IplImage* frameBuffer[frameCount];
    selectFrames("../View_008", frameBuffer, frameCount, 1);

    // On fait rouler la machine
    playLoop(frameBuffer, frameCount);
    
    return 0;
}
