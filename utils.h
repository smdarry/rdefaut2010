#ifndef _UTILS_H_
#define _UTILS_H_

#include "cv.h"
#include "highgui.h"

#include <stdio.h>

void printFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    // Print index row
    int i;
    for(i = 0; i < frame->width-1; i++)
        fprintf(fp, "%d,", i);
    fprintf(fp, "%d\n", frame->width-1);

    int row, col, step = frame->widthStep;
    for(row = 0; row < frame->height; row++)
    {
        for(col = 0; col < frame->width; col++)
        {
            float f = ((float*)(frame->imageData + step*row))[col*3+channel];
            fprintf(fp, "%.2f,", f);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void printByteFrame(IplImage* frame, int channel, char* filename)
{
    FILE* fp = fopen(filename, "w+");

    int row, col, step = frame->widthStep;
    for(row = 0; row < frame->height; row++)
    {
        for(col = 0; col < frame->width; col++)
        {
            uchar c = ((uchar*)(frame->imageData + step*row))[col*3+channel];
            fprintf(fp, "%d,", c);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void printPatch(IplImage* frame, int channel, char* filename, int beginX, int beginY, int endX, int endY)
{
    uchar pixels[endX - beginX];

    FILE* fp = fopen(filename, "w+");

    int i = 0, row, col, step = frame->widthStep;
    for(row = beginY; row <= endY; row++)
    {
        for(col = beginX; col <= endX; col++)
        {
            pixels[i] = ((uchar*)(frame->imageData + step*row))[col*3];
            fprintf(fp, "%d,", pixels[i]);
            i++;
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void printPatchF(IplImage* frame, int channel, char* filename, int beginX, int beginY, int endX, int endY)
{
    FILE* fp = fopen(filename, "w+");

    int i = 0, row, col, step = frame->widthStep;
    for(row = beginY; row <= endY; row++)
    {
        for(col = beginX; col <= endX; col++)
        {
            float f = ((float*)(frame->imageData + step*row))[col*3];
            fprintf(fp, "%.2f,", f);
            i++;
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void analysePixel(IplImage* frame, IplImage* mean, IplImage* sdv, int x, int y)
{
    float p[3], m[3], s[3], d[3], r[3];

    int ustep = frame->widthStep;
    int fstep = mean->widthStep;

    printf("\n** Analysis result for pixel [%d,%d] **\n", x, y);

    int channel;
    for(channel = 0; channel < 3; channel++)
    {
        p[channel] = (float)((uchar*)(frame->imageData + ustep*y))[x*3+channel];
        m[channel] = ((float*)(mean->imageData + fstep*y))[x*3+channel];
        s[channel] = ((float*)(sdv->imageData + fstep*y))[x*3+channel];
        d[channel] = fabs(p[channel] - m[channel]);
        r[channel] = fabs(d[channel] / s[channel]);
    }

    printf("Intensity: (%.2f, %.2f, %.2f)\n", p[0], p[1], p[2]);
    printf("Mean: (%.2f, %.2f, %.2f)\n", m[0], m[1], m[2]);
    printf("Standard dev.: (%.2f, %.2f, %.2f)\n", s[0], s[1], s[2]);
    printf("Diff.: (%.2f, %.2f, %.2f)\n", d[0], d[1], d[2]);
    printf("Diff/Std.: (%.2f, %.2f, %.2f)\n", r[0], r[1], r[2]);
}

IplConvKernel* createRectMask(int maskSize)
{
    int centerX = maskSize / 2;
    int centerY = centerX;

    // Cree un masque de la taille specifiee
    IplConvKernel* mask = cvCreateStructuringElementEx(maskSize, maskSize, 
                                                       centerX, centerY, 
                                                       CV_SHAPE_RECT, NULL);
    return mask;
}

void opening(IplImage* src, IplImage* dst, int maskSize)
{
    IplConvKernel* mask = createRectMask(maskSize);

    // Une ouverture est une erosion suivie d'une dilatation
    cvErode(src, dst, mask, 1);
    cvDilate(dst, dst, mask, 1);

    cvReleaseStructuringElement(&mask);
}

void closing(IplImage* src, IplImage* dst, int maskSize)
{
    IplConvKernel* mask = createRectMask(maskSize);

    // Une fermeture est une dilation suivie d'une erosion 
    cvDilate(src, dst, mask, 1);
    cvErode(dst, dst, mask, 1);

    cvReleaseStructuringElement(&mask);
}

void openSave(IplImage* frame, int maskSize, char* filename)
{
    IplImage* tmp = cvCloneImage(frame);

    opening(frame, tmp, maskSize);
    cvSaveImage(filename, tmp);

    cvReleaseImage(&tmp);
}

void openCloseSave(IplImage* frame, int maskSize, char* filename)
{
    IplImage* tmp = cvCloneImage(frame);

    opening(frame, tmp, maskSize);
    closing(tmp, tmp, maskSize);
    cvSaveImage(filename, tmp);

    cvReleaseImage(&tmp);
}

/**
 * TODO: Fonctionne sous la supposition que le frameBuffer sera toujours plein.
 */
void selectFrames(char* dir, IplImage* frameBuffer[], int frameCount, int interval)
{
    // Un buffer circulaire permet de ne garder que les n dernieres frames
    int frameBufIndex = 0;

    // Initialize les pointeurs d'image a NULL
    int i;
    for(i = 0; i < frameCount; i++)
    {
        frameBuffer[i] = NULL;
    }

    char filename[256];
    CvSize frameSize = cvSize(0, 0);

    IplImage* frame = NULL;
    for(i = 0; i < frameCount; i += interval)
    {
        sprintf(filename, "%s/frame_%04d.jpg", dir, i);

        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(frame == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }

        // Garde l'image selectionnee dans le buffer circulaire
        // en prenant soin de liberer l'image qui se fera bumper
        IplImage* outdatedFrame = frameBuffer[frameBufIndex];
        if(outdatedFrame != NULL)
            cvReleaseImage(&outdatedFrame);
        frameBuffer[frameBufIndex] = frame;
        frameBufIndex = (frameBufIndex + 1) % frameCount;
    }
}

void pickFrames(char* dir, IplImage* frameBuffer[], int frameBegin, int frameEnd)
{
    int frameBufIndex = 0;
    char filename[256];
    CvSize frameSize = cvSize(0, 0);

    IplImage* frame = NULL;
    int i;
    for(i = frameBegin; i <= frameEnd; i++)
    {
        sprintf(filename, "%s/frame_%04d.jpg", dir, i);

        frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
        if(frame == NULL)
        {
            fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
            continue;
        }
        frameBuffer[frameBufIndex] = frame;
        frameBufIndex++;
    }
}

#endif
