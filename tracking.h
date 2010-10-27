#ifndef _TRACKING_H_
#define _TRACKING_H_

typedef struct _track
{
    int label;
    TrackClass classif;

    float avgAspectRatio;

    int avgDirX;
    int avgDirY;
    float avgSpeed;

    CvSeq* historyTrail;
} Track;

typedef struct _metrics
{
    CvMat* mSpatial;
    CvMat* mHist5;
    CvMat* mHist10;
    CvMat* mHist15;
} DistMetrics;

void distanceMetrics(Blob* blobs, Blob* pBlobs, int blobCount, int pBlobCount, DistMetrics* m)
{
    // Matrice des combinaisons de recouvrements spatiaux
    m->mSpatial = cvCreateMat(blobCount, pBlobCount, CV_32FC1);

    // Matrices des differences d'histogramme
    m->mHist5 = cvCreateMat(blobCount, pBlobCount, CV_32FC3);
    m->mHist10 = cvCreateMat(blobCount, pBlobCount, CV_32FC3);
    m->mHist15 = cvCreateMat(blobCount, pBlobCount, CV_32FC3);

    float coverage, absDiff;
    Blob *b1, *b2;
    int step = m->mSpatial->step, hstep = m->mHist5->step;
    int b,pb;
    for(b = 0; b < blobCount; b++)
    {
        for(pb = 0; pb < pBlobCount; pb++)
        {
            b1 = &blobs[b];
            b2 = &pBlobs[pb];

            coverage = percentOverlap(b1, b2);
            ((float*)(m->mSpatial->data.ptr + b*step))[pb] = coverage;

            absDiff = absDiffHistograms(&b1->h5, &b2->h5, 0);
            ((float*)(m->mHist5->data.ptr + b*hstep))[pb*3] = absDiff;

            absDiff = absDiffHistograms(&b1->h10, &b2->h10, 0);
            ((float*)(m->mHist10->data.ptr + b*hstep))[pb*3] = absDiff;

            absDiff = absDiffHistograms(&b1->h15, &b2->h15, 0);
            ((float*)(m->mHist15->data.ptr + b*hstep))[pb*3] = absDiff;

            // TODO: Faire les autres canaux
        }
    }
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

void fusion(Blob* blobs, int blobCount, Blob* pBlobs, int pBlobCount)
{
    DistMetrics m;
    distanceMetrics(blobs, pBlobs, blobCount, pBlobCount, &m);

    int assocMatrix[blobCount];
    association(blobs, pBlobs, &m, assocMatrix);

    // Transfer des identites (etiquettes) aux nouveaux blobs
    int b;
    for(b = 0; b < blobCount; b++)
    {
        int index = assocMatrix[b];
        if(index != -1)
            blobs[b].label = pBlobs[index].label;
        else
            blobs[b].label = generateLabel();
    }

    // Evaluation de la velocite pour chaque blob
    for(b = 0; b < blobCount; b++)
    {
        int index = assocMatrix[b];
        if(index != -1)
            velocity(&pBlobs[index], &blobs[b]);
        else
            velocity(NULL, &blobs[b]);
    }
}

void classify(Blob* blobs, int blobCount)
{
    int b;
    for(b = 0; b < blobCount; b++)
    {
        if(blobs[b].avgSpeed > 10.0)
        {
            blobs[b].classif = VEHICLE;
        }
        else
        {
            if(blobs[b].aspectRatio < 0.8)
                blobs[b].classif = PERSON;
            else
                blobs[b].classif = VEHICLE;
        }
    }
}

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

#endif
