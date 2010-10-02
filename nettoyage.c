#include "cv.h"
#include "utils.h"

int main( int argc, char** argv )
{
    if(argc < 2)
    {
        printf("Veuillez specifier l'image a nettoyer...\n");
        return;
    }

    char* filename = argv[1];

    IplImage* frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    if(frame == NULL)
    {
        fprintf(stderr, "Erreur de lecture de l'image %s\n", filename);
    }

    //
    // a) une ouverture seule
    //
    openSave(frame, 3, "frameOpen3.jpg");
    openSave(frame, 5, "frameOpen5.jpg");

    //
    // b) une ouverture puis une fermeture
    //
    openCloseSave(frame, 3, "frameOpenClose3.jpg");
    openCloseSave(frame, 5, "frameOpenClose5.jpg");
}
