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

int cmpBlobs(const void* b1, const void* b2)
{
    CvSeq* s1 = (CvSeq*)b1;
    CvSeq* s2 = (CvSeq*)b2;

    return (s1->total - s2->total);
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

                if(label == 39)
                {
                    printf("pixel [%d]: %d,%d\n", label, col, row);
                }
            }
        }
    }

/*
    // Tri des blobs par nombre d'elements
    qsort(blobs, blobCount, sizeof(CvSeq*), cmpBlobs);

    int i;
    for(i = 0; i < blobs[0]->total; i++)
    {
        CvPoint* p = (CvPoint*)cvGetSeqElem(blobs[0], i);
        printf("Blob %d pixel: %d,%d\n", 0, p->x, p->y);
    }
*/

/*
    CvRect box;
    boundingBox(blobs[maxB], &box);

    // Affichage des boites
    CvPoint p1 = cvPoint(box.x, box.y);
    CvPoint p2 = cvPoint(box.x + box.width, box.y + box.height);

    cvRectangle(frame, p1, p2, CV_RGB(255,255,255), 1, 8, 0);
*/

/*
    // Calcul des boites englobantes
    CvRect box[blobCount];
    for(b = 0; b < blobCount; b++)
    {
        boundingBox(blobs[b], &box[b]);

        // Affichage des boites
        CvPoint p1 = cvPoint(box[b].x, box[b].y);
        CvPoint p2 = cvPoint(box[b].x + box[b].width, box[b].y + box[b].height);

        cvRectangle(frame, p1, p2, CV_RGB(255,255,255), 1, 8, 0);
    }
*/
    cvSaveImage("blobsImage.jpg", frame);

/*
    int i;
    for(i = 0; i < blobCount; i++)
    {
        boundingBox(blobs[i]);
    }
*/

    freeBlobs(storage);
    cvReleaseImage(&frame);
}
