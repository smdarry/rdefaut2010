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

int comparePoint(const void* a, const void* b, void* data)
{
    CvPoint* p1 = (CvPoint*)a;
    CvPoint* p2 = (CvPoint*)b;
   
    int dim = *((int*)data); 

    if(dim == 0)
        return p1->x > p2->x;

    return p1->y > p2->y;
}

inline void boundingBox(CvSeq* seq)
{
    // dimension: x = 0, y = 1
    int dim;

    // Premier tri en y
    dim = 1;
    cvSeqSort(seq, comparePoint, &dim);

    // Second tri en x
    dim = 0;
    cvSeqSort(seq, comparePoint, &dim);
}

int main( int argc, char** argv )
{
    const char* frameName = "../resultat.jpg";
    IplImage* frame = cvLoadImage(frameName, 0);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", frameName);
    }

    CvMat* matEtiq = cvCreateMatHeader(frame->height, frame->width, CV_32SC1);
    IplImage *imgEtiq = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);

    etiquetage((uchar*)frame->imageData, (int**)&matEtiq->data.ptr, 
                frame->width, frame->height);
    convertEtiq(&matEtiq, imgEtiq);

    cvSaveImage("imEtiq.pgm", imgEtiq);

    // Premiere passe pour determiner le nombre de blobs
    int label, row, col;
    int max = 0, step = matEtiq->step;
    for(row = 0 ; row < matEtiq->rows; row++)
    {
        for(col = 0; col < matEtiq->cols; col++)
        {
            label = ((int*)(matEtiq->data.ptr + matEtiq->cols*row))[col];
            if(label > max)
                max = label;
        }
    }
    printf("Max Label = %d\n", max);

    // Extraction des blobs
    // Allocation d'un vecteur de la taille determinee
    int blobCount = max;
    CvSeq* blobs[blobCount];
    CvMemStorage* storage = cvCreateMemStorage(0);

    initBlobs(blobs, blobCount, storage);
    
    // Deuxieme passe: extraction des blobs
    CvPoint* p;
    for(row = 0 ; row < matEtiq->rows; row++)
    {
        for(col = 0; col < matEtiq->cols; col++)
        {
            label = ((int*)(matEtiq->data.ptr + matEtiq->cols*row))[col];
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
    int i;
    for(i = 0; i < blobCount; i++)
    {
        //boundingBox(blobs[i]);
    }

    freeBlobs(storage);
}
