typedef struct _blob
{
    CvSeq* points;
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

void freePointSeqs(CvMemStorage* storage)
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
