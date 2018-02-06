#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "OutilsPGM.h"

#define Max 256

#define NBVoisinage 9

#define TailleNagao 5

typedef struct {
  int taille;
  float** filtre;
} Filtre;

int* TableauMedian;


int seuil = 125;

char path[100] = "Anonyme.pgm";

Filtre Moyenne;
Filtre Sobel;







Image img;
Image imgref;
Image imgM; // Image après filtre Moyenne
Image imgMedian; // Image après filtre Median
Image imgNagao; // Image après filtre Nagao
Image imgSobelx; // Image après filtre de Sobel
Image imgSobely; // Image après filtre Sobel
Image imgGradient; // Image après Gradient
Image imgSeuil; // Image après Gradient
Image imgExpan;

GLubyte* I; //Image initial


void SeuilGradient(Image* img, int seuil);


  void initGL(){
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    I=malloc((img.size)*sizeof(GLubyte));
  }

  void BasculeImage(Image* img, GLubyte* I){
    int i,j;
    for(i=0; i<img->height; i++)
      for(j=0; j<img->width; j++)
        I[(img->height-1-i)*img->width+j]=(GLubyte) img->data[i*img->width+j];
    return;
  }


  void redim(int width, int height){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width,0,height);
    glMatrixMode(GL_MODELVIEW);
  }

  void affichage (void){
    BasculeImage(&img, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(img.width, img.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }

  void affichagemoyenne (void){
    BasculeImage(&imgM, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(imgM.width, imgM.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }

  void affichageMedian (void){
    BasculeImage(&imgMedian, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(imgMedian.width, imgMedian.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }

  void affichageNagao (void){
    BasculeImage(&imgNagao, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(imgNagao.width, imgNagao.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }
  void affichageSobelx (void){
    BasculeImage(&imgSobelx, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(imgSobelx.width, imgSobelx.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }

  void affichageSobely (void){
    BasculeImage(&imgSobely, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(imgSobely.width, imgSobely.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }

  void affichageGradient (void){
    BasculeImage(&imgGradient, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(imgGradient.width, imgGradient.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }

  void affichageSeuil (void){
    BasculeImage(&imgSeuil, I);
    glClear (GL_COLOR_BUFFER_BIT);
    if (I!=NULL)
      glDrawPixels(imgSeuil.width, imgSeuil.height , GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
    glFlush();
  }

  void placer (int g, int d, int *T){
    int i, j, x, place_trouvee;
    x=T[g];
    i=g;
    j=2*g+1;
    place_trouvee=0;

    while ((j<=d) && !(place_trouvee))
    {
      if (j<d)
      if (T[j+1]>T[j]) j=j+1;

      if (x>=T[j]) place_trouvee=1;
      else{
        T[i]=T[j];
        i=j;
        j=2*i+1;
      }
    }
    T[i]=x;
  }

  void heapsort (int iMax, int t[iMax]){
   int g,d=iMax-1,echange;
   for (g=(iMax/2)+1; g>=0;g--)
   	placer(g,iMax-1,t);

   while (d>0){
  	 echange = t[d];
  	 t[d]=t[0];
  	 t[0]=echange;
  	 placer(0,--d,t);
   }
  }

  int CalculMedian (int n,int t[n]){
    heapsort(n, t);
    return t[n/2];
  }

  float CalculMoyenne (int t[NBVoisinage]){
    float s=0.0;
    int i;
    for (i=0; i<NBVoisinage ; i++)
      s += (float)t[i];
    return s/(float)(NBVoisinage-1);
  }

  float CalculVariance (int tv[NBVoisinage]){
    int i;
    float v = 0.0;
    float moy;
    moy = CalculMoyenne(tv);
    for (i=0; i<NBVoisinage ; i++)
      v += (tv[i]-moy)*(tv[i]-moy);
    return v/(float)NBVoisinage;
  }


  int findmin (float t[NBVoisinage]){
    float min = t[0];
    int indice;
    int i;
    for (i=0; i < NBVoisinage; i++){
      if (t[i]<min){
        min=t[i];
        indice = i;
      }
    }
    return indice;
  }

  void ExpansionDynamique (Image* img){
    int min = img->data[0];
    int max = img->data[0];
    int i;
    int x,y;
    int pos;
    float alpha,beta;

    for (y=0; y < img->height; y++)
      for (x=0; x < img->width; x++){
        pos = y*img->width+x;
        /*Expansion Dynamique*/
        if (img->data[pos] > max) max = img->data[pos];
        if (img->data[pos] < min) min = img->data[pos];}
    alpha = (Max-1) / (float) (max - min);
    beta = -(Max-1) * (float) min /(float) (max - min);
    for (y=0; y < img->height; y++)
      for (x=0; x < img->width; x++){
        pos = y*img->width+x;
        img->data[pos] = alpha * img->data[pos] + beta;
    }
  }


  void ApplicationFiltre(Filtre* filtre, Image* img){
    int x,y,i,j;
    int pos;
    float resultatfiltre = 0.0;

    for (y=0; y < img->height; y++){
      for (x=0; x < img->width; x++){
        pos = y*img->width+x;
        for (i= -filtre->taille / 2; i <= filtre->taille / 2; i++){
          for (j= -filtre->taille / 2; j <= filtre->taille / 2; j++){
            if (x+i < 0 && y+j < 0)
              resultatfiltre += imgref.data[(y-j)*img->width+(x-i)] * filtre->filtre[i + filtre->taille / 2][j + filtre->taille / 2];
            else if (x+i > img->width && y+j > img->height)
              resultatfiltre += imgref.data[(y-j)*img->width+(x-i)] * filtre->filtre[i + filtre->taille / 2][j + filtre->taille / 2];
            else if (x+i < 0 || x+i > img->width)
              resultatfiltre += imgref.data[(y+j)*img->width+(x-i)] * filtre->filtre[i + filtre->taille / 2][j + filtre->taille / 2];
            else if (y+j < 0 || y+j > img->height)
              resultatfiltre += imgref.data[(y-j)*img->width+(x+i)] * filtre->filtre[i + filtre->taille / 2][j + filtre->taille / 2];
            else
              resultatfiltre += imgref.data[(y+j)*img->width+(x+i)] * filtre->filtre[i + filtre->taille / 2][j + filtre->taille / 2];
          }
        }
        img->data[pos] = resultatfiltre;
        resultatfiltre = 0.0;
      }
    }
  }


  void FiltreMoyenne(Filtre* Moyenne, int tai){
    int i,j;
    Moyenne->taille=tai;

    Moyenne->filtre = malloc( (Moyenne->taille) * sizeof(float));
    for (i=0; i< Moyenne->taille; i++)
      Moyenne->filtre[i] = malloc( (Moyenne->taille) * sizeof(float));

    for (i=0; i < Moyenne->taille; i++)
      for (j=0; j< Moyenne->taille; j++)
        Moyenne->filtre[i][j] = 1.0 / (float) (Moyenne->taille*Moyenne->taille );

    ApplicationFiltre(Moyenne, &imgM);

    glutInitWindowSize (img.width, img.height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image après Moyenne");
    glutDisplayFunc(affichagemoyenne);
    glutReshapeFunc (redim);

  }
  void keyboardGL(unsigned char k, int x, int y){
    if (k == '+' && seuil < 255){ seuil += 5; SeuilGradient(&imgSeuil, seuil); printf("%d\n",seuil );}
    if (k == '-'&& seuil > 0){ seuil -= 5; SeuilGradient(&imgSeuil, seuil); printf("%d\n",seuil );}
    glutPostRedisplay();
  }

  void SeuilGradient(Image* img, int seuil){
    int x,y;
    int pos;

    for (y=0; y < img->height; y++)
      for (x=0; x < img->width; x++){
        pos = y*img->width+x;
        if(imgGradient.data[pos] > seuil) img->data[pos] = 255;
        else img->data[pos] = 0;
      }
          glutKeyboardFunc(keyboardGL);
          glutDisplayFunc(affichageSeuil);
  }

  void Gradient (Image imgSobelx, Image imgSobely){
    int i,j;
    for(i=0; i < imgSobelx.height; i++)
    for (j=0; j < imgSobelx.width; j++)
    imgGradient.data[i*imgSobelx.width+j] = sqrt( pow(imgSobelx.data[i*imgSobelx.width+j],2) + pow(imgSobely.data[i*imgSobelx.width+j],2));

    SeuilGradient(&imgSeuil,seuil);
    glutInitWindowSize (imgSeuil.width, imgSeuil.height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image");
    glutDisplayFunc(affichage);
    glutReshapeFunc (redim);

    glutInitWindowSize (imgGradient.width, imgGradient.height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image Gradient");
    glutDisplayFunc(affichageGradient);
    glutReshapeFunc (redim);


  }

  void FiltreSobel(Filtre* Sobel){
    int i,j;
    Sobel->taille=3;

    Sobel->filtre = malloc( (Sobel->taille) * sizeof(int));
    for (i=0; i < Sobel->taille; i++)
      Sobel->filtre[i] = malloc( (Sobel->taille) * sizeof(int));

	Sobel->filtre[0][0] = 1.0;
	Sobel->filtre[0][1] = 0.0;
	Sobel->filtre[0][2] = -1.0;
	Sobel->filtre[1][0] = 2.0;
	Sobel->filtre[1][1] = 0.0;
	Sobel->filtre[1][2] = -2.0;
	Sobel->filtre[2][0] = 1.0;
	Sobel->filtre[2][1] = 0.0;
	Sobel->filtre[2][2] = -1.0;

    ApplicationFiltre(Sobel, &imgSobelx);

    Sobel->filtre[0][0] = 1;
    Sobel->filtre[0][1] = 2;
    Sobel->filtre[0][2] = 1;
    Sobel->filtre[1][0] = 0;
    Sobel->filtre[1][1] = 0;
    Sobel->filtre[1][2] = 0;
    Sobel->filtre[2][0] = -1;
    Sobel->filtre[2][1] = -2;
    Sobel->filtre[2][2] = -1;

    ApplicationFiltre(Sobel, &imgSobely);

    Gradient(imgSobelx,imgSobely);

    ExpansionDynamique(&imgSobelx);
    ExpansionDynamique(&imgSobely);

    glutInitWindowSize (img.width, img.height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image après Sobel (Dx)");
    glutDisplayFunc(affichageSobelx);
    glutReshapeFunc (redim);

    glutInitWindowSize (img.width, img.height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image après Sobel (Dy)");
    glutDisplayFunc(affichageSobely);
    glutReshapeFunc (redim);
  }


  void FiltreMedian(int tai, Image* img){
    int x,y,i,j;
    int pos;
    TableauMedian = malloc ( (tai*tai) * sizeof(int) );

        for (y=0; y < img->height; y++){
          for (x=0; x < img->width; x++){
            pos = y*img->width+x;
            for (i= -tai / 2; i <= tai / 2; i++){
              for (j=-tai / 2; j <= tai / 2; j++){
                if (x+i < 0 && y+j < 0)
                  TableauMedian[(i + tai / 2) * tai + (j + tai / 2) ] = img->data[(y-j)*img->width+(x-i)];
                else if (x+i > img->width && y+j > img->height)
                  TableauMedian[(i + tai / 2) * tai + (j + tai / 2) ] = img->data[(y-j)*img->width+(x-i)];
                else if (x+i < 0 || x+i > img->width)
                  TableauMedian[(i + tai / 2) * tai + (j + tai / 2) ] = img->data[(y+j)*img->width+(x-i)];
                else if (y+j < 0 || y+j > img->height)
                  TableauMedian[(i + tai / 2) * tai + (j + tai / 2) ] = img->data[(y-j)*img->width+(x+i)];
                else
                  TableauMedian[(i + tai / 2) * tai + (j + tai / 2) ] = img->data[(y+j)*img->width+(x+i)];
              }
            }
              img->data[pos] = CalculMedian(tai*tai, TableauMedian);
          }
        }

    glutInitWindowSize (img->width, img->height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image après Medianne");
    glutDisplayFunc(affichageMedian);
    glutReshapeFunc (redim);
  }

  void FiltreNagao(Image* img){

    int Nagao[TailleNagao*TailleNagao];
    int TableauNagao[NBVoisinage];
    float TableauVar[NBVoisinage];
    int TableauMed[NBVoisinage];
    int x,y,i,j,decali,decalj;
    int pos;

    for (y=0; y < img->height; y++){
      for (x=0; x < img->width; x++){
        pos = y*img->width+x;
        for (i= -TailleNagao / 2; i <= TailleNagao / 2; i++){
          for (j=-TailleNagao / 2; j <= TailleNagao / 2; j++){
            if (x+i < 0 && y+j < 0)
              Nagao[(i + TailleNagao / 2) * TailleNagao + (j + TailleNagao / 2) ] = img->data[(y-j)*img->width+(x-i)];
            else if (x+i > img->width && y+j > img->height)
              Nagao[(i + TailleNagao / 2) * TailleNagao + (j + TailleNagao / 2) ] = img->data[(y-j)*img->width+(x-i)];
            else if (x+i < 0 || x+i > img->width)
              Nagao[(i + TailleNagao / 2) * TailleNagao + (j + TailleNagao / 2) ] = img->data[(y+j)*img->width+(x-i)];
            else if (y+j < 0 || y+j > img->height)
              Nagao[(i + TailleNagao / 2) * TailleNagao + (j + TailleNagao / 2) ] = img->data[(y-j)*img->width+(x+i)];
            else
              Nagao[(i + TailleNagao / 2) * TailleNagao + (j + TailleNagao / 2) ] = img->data[(y+j)*img->width+(x+i)];
          }
        }
        for (decali=0; decali < 3; decali++)
          for (decalj=0; decalj < 3; decalj++){
            for (i=0; i < 3; i++)
              for (j=0; j < 3; j++)
                TableauNagao[i*3+j] = Nagao[ (i+decali) * 3 + (j+decalj) ];
          TableauVar[decali*3+decalj] = CalculVariance(TableauNagao);
          TableauMed[decali*3+decalj] = CalculMedian(NBVoisinage, TableauNagao);
          printf("%f : %d \n",TableauVar[decali*3+decalj],decali*3+decalj);
          }
          img->data[pos] = TableauMed[findmin(TableauVar)];

      }
    }


    glutInitWindowSize (img->width, img->height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image après Nagao");
    glutDisplayFunc(affichageNagao);
    glutReshapeFunc (redim);
  }


  void menu(int value){
    switch (value){
      case 0 : exit(0); break;
      case 1 : FiltreMoyenne(&Moyenne,3); break;
      case 2 : FiltreMoyenne(&Moyenne,5); break;
      case 3 : FiltreMoyenne(&Moyenne,7); break;
      case 4 : FiltreMedian(3,&imgMedian); break;
      case 5 : FiltreMedian(5,&imgMedian); break;
      case 6 : FiltreMedian(7,&imgMedian); break;
      case 7 : FiltreNagao(&imgNagao); break;
      case 8 : FiltreSobel(&Sobel); break;
      default : break;
    }
    glutPostRedisplay();
  }

  int main (int argc, char **argv){

    LireImage(path, &img);
    LireImage(path, &imgref);
    LireImage(path, &imgM);
    LireImage(path, &imgMedian);
    LireImage(path, &imgNagao);
    LireImage(path, &imgSobely);
    LireImage(path, &imgSobelx);
    LireImage(path, &imgGradient);
    LireImage(path, &imgSeuil);



    glutInit(&argc,argv);
    initGL();

    glutInitWindowSize (img.width, img.height);
    glutInitWindowPosition (200,100);
    glutInitDisplayMode (GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow ("Image");
    glutDisplayFunc(affichage);
    glutReshapeFunc (redim);




    glutCreateMenu(menu);
    glutAddMenuEntry("Filtre Sobel",8);
    glutAddMenuEntry("Filtre Nagao",7);
    glutAddMenuEntry("Filtre Median 7x7",6);
    glutAddMenuEntry("Filtre Median 5x5",5);
    glutAddMenuEntry("Filtre Median 3x3",4);
    glutAddMenuEntry("Filtre Moyenne 7x7",3);
    glutAddMenuEntry("Filtre Moyenne 5x5",2);
    glutAddMenuEntry("Filtre Moyenne 3x3",1);
    glutAddMenuEntry("Quitter",0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);





    glutMainLoop ();

    return 0;
  }
