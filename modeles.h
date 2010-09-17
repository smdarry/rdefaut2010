/** 
 * Le modele Gaussien est represente par deux images a 3 channels. L'une des 
 * images contient la valeur moyenne, et l'autre, l'ecart-type.
 */
typedef struct _modeleGaussien
{
    IplImage* mean;
    IplImage* stdDev;
} ModeleGaussien;

/** 
 * Le modele median est represente par une image a 3 channels. Chacune contient
 * la valeur medianne a chaque pixel pour chaque couleur.
 */
typedef struct _modeleMedian
{
    IplImage* median;
} ModeleMedian;

void initModeleMedian(ModeleMedian* model, IplImage* frame)
{
     model->median = cvCreateImage(cvSize(frame->width, frame->height), 
                                    IPL_DEPTH_32F, 3);
    cvSetZero(model->median);
}

void initModeleGaussien(ModeleGaussien* model, IplImage* frame)
{
     model->mean = cvCreateImage(cvSize(frame->width, frame->height), 
                                    IPL_DEPTH_32F, 3);
    cvSetZero(model->mean);

     model->stdDev = cvCreateImage(cvSize(frame->width, frame->height), 
                                    IPL_DEPTH_32F, 3);
    cvSetZero(model->stdDev);
}
