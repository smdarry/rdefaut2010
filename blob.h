#include "cv.h"

#include "histogram.h"
#include "etiquette.h"

#define BLOB_SIZE 200

typedef struct _blob
{
    int label;

    CvSeq* points;
    CvMemStorage* storage;

    CvRect box;

    Histogram h5;
    Histogram h10;
    Histogram h15;
} Blob;

void initPointSeqs(CvSeq* seqs[], int length, CvMemStorage* storage)
{
    int i;
    for(i = 0; i < length; i++)
    {
        seqs[i] = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), 
                               sizeof(CvPoint), storage);
    }
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

inline void boundingBox(Blob* blob)
{
    CvSeq* seq = blob->points;

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

    // L'origine du restangle = point superieur gauche
    blob->box.x = minX;
    blob->box.y = minY;
    blob->box.width = maxX - minX;
    blob->box.height = maxY - minY;
}

int areOverlapping(Blob* b1, Blob* b2)
{
    const int r1_x1 = b1->box.x, r1_x2 = b1->box.x + b1->box.width;
    const int r2_x1 = b2->box.x, r2_x2 = b2->box.x + b2->box.width;
    
    const int r1_y1 = b1->box.y, r1_y2 = b1->box.y + b1->box.height;
    const int r2_y1 = b2->box.y, r2_y2 = b2->box.y + b2->box.height;

    if(r1_x1 < r2_x2 && r1_x2 > r2_x1 && 
       r1_y1 < r2_y2 && r1_y2 > r2_y1)
        return 1;

    return 0;
}

int cmpCoords(const void *c1, const void *c2)
{
    return *((const int*)c1) - *((const int*)c2);
}

float percentOverlap(Blob* b1, Blob* b2)
{
    // Se chevauchent-ils?
    if(!areOverlapping(b1, b2))
    {
        return 0.0;
    }
      
    // Tri des X
    int xCoords[] = {b1->box.x, b1->box.x + b1->box.width, b2->box.x, b2->box.x + b2->box.width };
    qsort(xCoords, 4, sizeof(int), cmpCoords);
      
    // Tri des Y
    int yCoords[] = {b1->box.y, b1->box.y + b1->box.height, b2->box.y, b2->box.y + b2->box.height };
    qsort(yCoords, 4, sizeof(int), cmpCoords);
      
    // Determination de l'aire en commun
    float intArea = (xCoords[2] - xCoords[1]) * (yCoords[2] - yCoords[1]);

    // Aire du premier blob
    float b1Area = b1->box.height * b1->box.width;

    // Pourcentage de l'aire du premier blob couvert par le second
    return (intArea / b1Area);
}

void buildHistograms(Blob* blob, IplImage* frame)
{
    initHistogram(&blob->h5, 5, 3);
    initHistogram(&blob->h10, 10, 3);
    initHistogram(&blob->h15, 15, 3);

    int i;
    for(i = 0; i < blob->points->total; i++)
    {
        CvPoint* p = (CvPoint*)cvGetSeqElem(blob->points, i);

        updateHistogram(&blob->h5, frame, p->x, p->y);
        updateHistogram(&blob->h10, frame, p->x, p->y);
        updateHistogram(&blob->h15, frame, p->x, p->y);
    }
}

int extractBlobs(IplImage* binFrame, IplImage* colorFrame, Blob** blobs)
{    
    // Pre-Filtrage pour enlever les valeurs dues a la compression de l'image
    cvThreshold(binFrame, binFrame, 200, 255, CV_THRESH_BINARY);

    // Extraction des blobs
    CvMat* matEtiq = cvCreateMatHeader(binFrame->height, binFrame->width, 
                                       CV_32SC1);
    int blobCount = etiquetage((uchar*)binFrame->imageData, 
                               (int**)&matEtiq->data.i, binFrame->width, 
                               binFrame->height);

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
    cvReleaseMat(&matEtiq);

    // On ne garde que les "gros" blobs
    CvSeq* bigBlobPoints[blobCount];
    int b, bigBlobCount = 0;
    CvSeq* points;
    for(b = 0; b < blobCount; b++)
    {
        points = blobPoints[b];
        if(points->total > BLOB_SIZE)
        {
            bigBlobPoints[bigBlobCount++] = points;
        }
    }

    // Allocation d'espace pour heberger les blobs
    Blob* newBlobs = (Blob*)malloc(bigBlobCount * sizeof(Blob));

    // Assigne la liste de points au blob correspondant
    for(b = 0; b < bigBlobCount; b++)
    {
        newBlobs[b].label = -10;
        newBlobs[b].points = bigBlobPoints[b];
        newBlobs[b].storage = storage;
    }

    // Calcul des boites englobantes
    for(b = 0; b < bigBlobCount; b++)
    {
        boundingBox(&newBlobs[b]);
    }

    /*
    // Calcul des histogrammes
    for(b = 0; b < blobCount; b++)
    {
        buildHistograms(&newBlobs[b], colorFrame);

        normalizeHistogram(&newBlobs[b].h5);
        normalizeHistogram(&newBlobs[b].h10);
        normalizeHistogram(&newBlobs[b].h15);
    }
    */

    // Liberation de la memoire
    cvReleaseMemStorage(&storage);

    *blobs = newBlobs;

    return bigBlobCount;
}

void releaseBlobs(Blob* blobs)
{
    free(blobs);
}

void drawBoundingRects(IplImage* frame, Blob* blobs, int blobCount)
{
    Blob* blob;
    int b;
    for(b = 0; b < blobCount; b++)
    {
        blob = &blobs[b];
        boundingBox(blob);

        // Affichage des boites
        CvPoint p1 = cvPoint(blob->box.x, blob->box.y);
        CvPoint p2 = cvPoint(blob->box.x + blob->box.width, blob->box.y + blob->box.height);

        cvRectangle(frame, p1, p2, CV_RGB(255,255,255), 1, 8, 0);
    }
}

void drawLabels(IplImage* frame, Blob* blobs, int blobCount)
{
    Blob* blob;
    char label[255];

    int b;
    for(b = 0; b < blobCount; b++)
    {
        blob = &blobs[b];

        CvPoint p1 = cvPoint(blob->box.x, blob->box.y);

        sprintf(label, "P%02d", blob->label);

        // Affichage d'une etiquette sur chaque boite
        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
        cvPutText(frame, label, p1, &font, cvScalar(255, 255, 255, 0));
    }
}
