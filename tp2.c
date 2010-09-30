#include "cv.h"
#include "highgui.h"
#include "etiquette.h"

#include <stdio.h>

void initBlobs(CvSeq* blobs[], int length, CvMemStorage* storage)
{
    int i;
    for(i = 0; i < length; i++)
    {
        blobs[i] = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), 
                               sizeof(CvPoint), storage);
    }
}

void freeBlobs(CvMemStorage* storage)
{
    // TODO: J'assume que chaque point va etre automatiquement libere!
    cvReleaseMemStorage(&storage);
}

int isSmaller(const void* a, const void* b, void* data)
{
    CvPoint* p1 = (CvPoint*)a;
    CvPoint* p2 = (CvPoint*)b;
   
    int dim = *((int*)data); 
    
    if(dim == 0)
        return (p1->x - p2->x);

    return (p1->y - p2->y);
}

inline void boundingBox(CvSeq* seq, CvRect* box)
{
    int minX, maxX, minY, maxY;

    // dimension: x = 0, y = 1
    int dim;

    // Premier tri en y
    dim = 1;
    cvSeqSort(seq, isSmaller, &dim);

    minY = ((CvPoint*)cvGetSeqElem(seq, 0))->y;
    maxY = ((CvPoint*)cvGetSeqElem(seq, seq->total-1))->y;
    
    // Second tri en x
    dim = 0;
    cvSeqSort(seq, isSmaller, &dim);

    minX = ((CvPoint*)cvGetSeqElem(seq, 0))->x;
    maxX = ((CvPoint*)cvGetSeqElem(seq, seq->total-1))->x;

/*
    int i;
    for(i = 0; i < seq->total; i++)
    {
        CvPoint* p = (CvPoint*)cvGetSeqElem(seq, i);
        printf("Element: %d,%d\n", p->x, p->y);
    }
*/

    // L'origine du restangle = point superieur gauche
    box->x = minX;
    box->y = minY;
    box->width = maxX - minX;
    box->height = maxY - minY;
}

int main( int argc, char** argv )
{
    const char* frameName = "../resultat.jpg";
    IplImage* frame = cvLoadImage(frameName, 0);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", frameName);
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

    // Allocation d'un vecteur de la taille determinee
    CvSeq* blobs[blobCount];
    CvMemStorage* storage = cvCreateMemStorage(0);

    initBlobs(blobs, blobCount, storage);
    
    // Collection des pixels de chaque blob
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

                cvSeqPush(blobs[label-1], p);
            }
        }
    }

    // Calcul des boites englobantes
    int b;
    CvRect box[blobCount];
    for(b = 0; b < blobCount; b++)
    {
        boundingBox(blobs[b], &box[b]);

        // Affichage des boites
        CvPoint p1 = cvPoint(box[b].x, box[b].y);
        CvPoint p2 = cvPoint(box[b].x + box[b].width, box[b].y + box[b].height);

        cvRectangle(frame, p1, p2, CV_RGB(255,255,255), 1, 8, 0);
    }
    cvSaveImage("blobsImage.jpg", frame);

    freeBlobs(storage);
    cvReleaseImage(&frame);
}
