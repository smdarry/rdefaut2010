#include "cv.h"
#include "highgui.h"
#include "etiquette.h"
#include "histogram.h"
#include "blob.h"

#include <stdio.h>

int main( int argc, char** argv )
{
    const char* frameName = "../resultat.jpg";
    IplImage* frame = cvLoadImage(frameName, 0);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", frameName);
        return;
    }

    // Filtrage pour enlever les valeurs dues a la compression de l'image
    cvThreshold(frame, frame, 200, 255, CV_THRESH_BINARY);

    // Extraction des blobs
    CvMat* matEtiq = cvCreateMatHeader(frame->height, frame->width, CV_32SC1);
    IplImage *imgEtiq = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    int blobCount = etiquetage((uchar*)frame->imageData, 
                               (int**)&matEtiq->data.ptr, frame->width, 
                               frame->height);
    printf("%d blobs trouves\n", blobCount);
    cvReleaseImage(&imgEtiq);

    Blob blobs[blobCount];

    // Separation des pixels de chaque blob dans des listes chainees
    CvSeq* blobPoints[blobCount];
    CvMemStorage* storage = cvCreateMemStorage(0);

    initPointSeqs(blobPoints, blobCount, storage);
    
    CvPoint* p;
    int row, col, label;
    for(row = 0 ; row < matEtiq->rows; row++)
    {
        for(col = 0; col < matEtiq->cols; col++)
        {
            label = ((int*)(matEtiq->data.i + matEtiq->cols*row))[col];
            if(label > 0)
            {
                // Ajoute un point dans la bonne liste
                p = (CvPoint*)malloc(sizeof(CvPoint));
                p->x = col; p->y = row;

                cvSeqPush(blobPoints[label-1], p);
            }
        }
    }

    // Assigne la liste de points au blob correspondant
    int b;
    for(b = 0; b < blobCount; b++)
        blobs[b].points = blobPoints[b];

    // Calcul des boites englobantes
    for(b = 0; b < blobCount; b++)
    {
        Blob blob = blobs[b];
        boundingBox(&blob);

        // Affichage des boites
        CvPoint p1 = cvPoint(blob.box.x, blob.box.y);
        CvPoint p2 = cvPoint(blob.box.x + blob.box.width, blob.box.y + blob.box.height);

        cvRectangle(frame, p1, p2, CV_RGB(255,255,255), 1, 8, 0);
    }
    cvSaveImage("blobsImage.jpg", frame);

    // Charge la frame originale (couleur) pour calculer l'histogramme
    const char* frameOrigName = "../View_008/frame_0061.jpg";
    IplImage* frameOrig = cvLoadImage(frameOrigName, CV_LOAD_IMAGE_COLOR);
    if(frameOrig == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", frameOrigName);
        return;
    }

    // Construction de 3 histogrammes pour chaque blob
    Blob* blob;

    cvReleaseMemStorage(&storage);
    cvReleaseImage(&frame);
    cvReleaseImage(&frameOrig);
}
