/** 
 * Le modele Gaussien est represente par deux images a 3 channels. L'une des 
 * images contient la valeur moyenne, et l'autre, l'ecart-type.
 */
typedef struct _modeleGaussien
{
    IplImage* mean;
    IplImage* stdDev;
} GaussianModel;

/** 
 * Le modele median est represente par une image a 3 channels. Chacune contient
 * la valeur medianne a chaque pixel pour chaque couleur.
 */
typedef struct _modeleMedian
{
    IplImage* median;
} MedianModel;

void initMedianModel(MedianModel* model, CvSize size)
{
     model->median = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->median);
}

void initGaussianModel(GaussianModel* model, CvSize size)
{
     model->mean = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->mean);

     model->stdDev = cvCreateImage(size, IPL_DEPTH_32F, 3);
    cvZero(model->stdDev);
}

void releaseMedianModel(MedianModel* model)
{
    cvReleaseImage(&model->median);
}

void releaseGaussianModel(GaussianModel* model)
{
    cvReleaseImage(&model->mean);
    cvReleaseImage(&model->stdDev);
}
