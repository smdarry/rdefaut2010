#include "etiquette.h"

#include <stdlib.h>
#include <stdio.h>

// Etiquetage d'une image binaire (0=noir, 255=blanc)
//        pix2
// pix1  pixCur
int etiquetage(uchar* data, int** matEtiq2, int width, int height)
{
  int i, j;
  int nl, nc, numEtiq, minEtiq, maxEtiq, nbMaxEtiq, a, k;
  int pix1, pix2;
  int *T;
  int nbEtiq;
	int* matEtiq=0;
  nl = height;
  nc = width;
	minEtiq=0;
	maxEtiq=nc*nl;

  //cout << "nl=" << nl << " nc=" << nc << " nz=" << imageBin->nChannels << endl;

  numEtiq = 1;


  // Table de correspondance initialisee a T(i)=i
  nbMaxEtiq = nl*nc;
  matEtiq = (int*)malloc(nbMaxEtiq*sizeof(int));
	*matEtiq2=matEtiq;
  T=(int*)malloc(nbMaxEtiq*sizeof(int));
  for(i = 0; i < nbMaxEtiq; i++)
	{
    T[i] = i;
	//matEtiq[i]=0;
	}
	memset(matEtiq,0,nbMaxEtiq*sizeof(int));
  //printf("initialisation etiquetage OK %d %d\n",matEtiq,T);
	if(matEtiq==0)
		printf("Erreur d'allocation de matEtiq\n");
  // Premier balayage
  for(i = 1; i < nl; i++)
  {
    for(j = 1; j < nc; j++)
    {
	//printf(" pixel : %d %d %d\n",i,j,nc);
      // Si pixel objet en mouvement
      if( (data[(i*nc + j)]) != 0) {

		
	pix1 = matEtiq[(i*nc + j-1)];
	pix2 = matEtiq[(i-1)*nc + j];

	// si predecesseurs = fond alors nouvelle etiquette
	if( pix1==0 && pix2==0 ) {
 	  matEtiq[(i*nc + j)] = numEtiq;
	  numEtiq++;
	}
	else {
	  // Tous les predecesseurs ont une meme etiquette => pixCourant = meme etiquette aussi
	  if(pix1==pix2) {
	    matEtiq[(i*nc + j)] = pix1;
	  }
	  else {
	    // On cherche la plus petite etiquette des predecesseurs non nulle
	    if( pix1<pix2 ) {
	      if( pix1!=0 ) {
		minEtiq = pix1; maxEtiq = pix2;
	      }
	      else {
		minEtiq = pix2; maxEtiq = pix1;
	      }
	    }
	    else {
	      if( pix2!=0 ) {
		minEtiq = pix2; maxEtiq = pix1;
	      }
	      else {
		minEtiq = pix1; maxEtiq = pix2;
	      }
	    }

	    //printf(" Affecter T(minEtiq) au pix courant %d %d",minEtiq,maxEtiq);
	    matEtiq[i*nc + j] = T[minEtiq];
	    T[maxEtiq] = minEtiq;
		//printf(" ok\n");
	  }
	}
      }
      else
	matEtiq[i*nc + j]= 0;
    }
  }

//   cout << " AVANT : " << endl;
//   for( i = 1; i <= numEtiq; i++)
//     cout << "  " << T[i] ;

  //printf("Mise a jour de la table de correspondance\n");
  for( i = 1; i <= numEtiq; i++) {
    j = i;
    while(T[j] != j)
      {
	j = T[j];
      }
    T[i]=j;
  }    
//   cout << endl << " APRES : " << endl;
//   for( i = 1; i <= numEtiq; i++)
//     cout << "  " << T[i] ;

  // on donne des numeros d"etiquette qui se suivent
  int m, n=1;
  i=1;
  while(i<=numEtiq)
    {
      if(T[i]<=n)
	i++;
      else {

	// Nouvelle etiquette
	m = T[i];
	n++;
	for( j = i; j <= numEtiq; j++) 
	  if(T[j]==m)
	    T[j]=n;
	i++;
      }
    }

  nbEtiq = n-1;
//   cout << endl << " APRES : n=" << nbEtiq << endl;
//   for( i = 1; i <= numEtiq; i++)
//     cout << "  " << T[i] ;

	//printf("deuxieme passage\n");
  // Deuxieme balayage pour etiquetage final
  for(i = 1; i < nl; i++)
  {
    for(j = 1; j < nc; j++)
    { 
      // Si pixel objet en mouvement
      if( matEtiq[i*nc + j] != 0) {
	matEtiq[i*nc + j] = T[(int)matEtiq[i*nc + j]];
      }
    }
  }



  //delete[] T;
	free(T);
  return nbEtiq;
}

void convertEtiq(CvMat** matEtiq2, IplImage* imgEtiq)
{ 
  int i, j, nl, nc;
	
	CvMat* matEtiq;
	matEtiq=*matEtiq2;
  //imgEtiq = cvCreateImage(cvGetSize(matEtiq), IPL_DEPTH_8U, 1);
  nl = imgEtiq->height;
  nc = imgEtiq->width;
  for(i = 1; i < nl; i++)
    for(j = 1; j < nc; j++)
      *(imgEtiq->imageData + i*nc + j) = (unsigned char)((*(matEtiq->data.i + i*nc + j)) %256);
}

//========================================================
// Programme principal
//========================================================
/*
int main()
{
  IplImage *img;
  CvMat *matEtiq;
  IplImage *imgEtiq;

 // cvNamedWindow("Image",1);

  img = cvLoadImage("im.pgm",0); // 0 pour 1 channel
 
  etiquetage(img->imageData, matEtiq);
  convertEtiq(matEtiq, imgEtiq);

  cvSaveImage("imEtiq.pgm", imgEtiq);
  cvSaveImage("im2.pgm", img);
  cvSaveImage("imEtiq.bmp", imgEtiq);

  //cvShowImage("Image",imgEtiq);


  //cvWaitKey(0);
 // cvDestroyWindow("Image");
  cvReleaseImage(&img);
  cvReleaseImage(&imgEtiq);
  cvReleaseMat(&matEtiq);

  return 0;
}
*/
