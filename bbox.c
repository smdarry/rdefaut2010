#include "cv.h"
#include "blob.h"

int main( int argc, char** argv )
{
    if(argc < 2)
    {
        printf("Veuillez specifier l'image binaire...\n");
        return;
    }

    char* filename = argv[1];

    IplImage* frame = cvLoadImage(filename, 0);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
    }

    Blob* blobs;
    int blobCount = extractBlobs(frame, NULL, &blobs);
    
    printf("%d blobs trouves\n", blobCount);

    int b;
    for(b = 0; b < blobCount; b++)
    {
        printf("Label = %d\n", blobs[b].label);
        printf("x = %d\n", blobs[b].box.x);
    }

    // Dessine les boites englobantes
    drawBoundingRects(frame, blobs, blobCount);
    cvSaveImage("bbox-output.pgm", frame);

    releaseBlobs(blobs);
}
