#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "OutilsPGM.h"

char path[100] = "lena.pgm";

typedef struct Filter
{
  int size;
  double* values;
} Filter;

typedef struct Vec2
{
  double x;
  double y;
} Vec2;

Image image;
Image image_test_in;
Image image_test_gauss;
Image image_test_heat;

GLubyte *I; //Image de travail

Filter laplace;

void initLaplaceFilter()
{
  laplace.size = 9;
  laplace.values = malloc(laplace.size * sizeof(double));

  laplace.values[0] = 0;
  laplace.values[1] = 1;
  laplace.values[2] = 0;
  laplace.values[3] = 1;
  laplace.values[4] = -4;
  laplace.values[5] = 1;
  laplace.values[6] = 0;
  laplace.values[7] = 1;
  laplace.values[8] = 0;
}

void initGL()
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  I = malloc( (image.height * image.width) * sizeof(GLubyte) );
}

void BasculeImage(Image *img, GLubyte *I)
{
  int i, j;
  for (i = 0; i < img->height; i++)
    for (j = 0; j < img->width; j++)
      I[(img->height - 1 - i) * img->width + j] = (GLubyte) img->data[i * img->width + j];
  return;
}

void redim(int width, int height)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glMatrixMode(GL_MODELVIEW);
}

void affichage_h(void)
{
  BasculeImage(&image_test_heat, I);
  glClear(GL_COLOR_BUFFER_BIT);
  if (I != NULL)
    glDrawPixels(image.width, image.height, GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
  glFlush();
}

void affichage_g(void)
{
  BasculeImage(&image_test_gauss, I);
  glClear(GL_COLOR_BUFFER_BIT);
  if (I != NULL)
    glDrawPixels(image.width, image.height, GL_LUMINANCE, GL_UNSIGNED_BYTE, I);
  glFlush();
}


void LinearFilter(Filter* filter, Image* image_in, Image* image_out)
{
  int x, y, i, j;
  int pos;
  int k;

  int rows = image_in->height;
  int cols = image_in->width;

  int f_size;

  double filter_result;

  filter_result = 0.0;
  k = 0;
  f_size = (int) sqrt(filter->size) / 2; 

  for (y = 0; y < rows; y++)
    for (x = 0; x < cols; x++)
    {
    pos = y * cols + x;
    for (j = -f_size ; j <= f_size; j++)
      for (i = -f_size; i <= f_size; i++)
      {
        // Top angles borders
        if (y + j > rows && (x + i < 0 || x + i > cols))
          filter_result += image_in->data[(y - j) * cols + (x - i)] * filter->values[k];

        // Bottom angles borders
        else if (y + j < 0 && (x + i < 0 || x + i > cols))
          filter_result += image_in->data[(y - j) * cols + (x - i)] * filter->values[k];
      
        // Left & Right borders
        else if (x + i < 0 || x + i > cols)
          filter_result += image_in->data[(y + j) * cols + (x - i)] * filter->values[k];

        // Top & Bottom borders
        else if (y + j < 0 || y + j > rows)
          filter_result += image_in->data[(y - j) * cols + (x + i)] * filter->values[k];

        else
          filter_result += image_in->data[(y + j) * cols + (x + i)] * filter->values[k];
        
        k++;
      }
    image_out->data[pos] = filter_result;
    filter_result = 0.0;
    k=0;
    }
}

void Heat(Image* I, Image* O, double dt, double D, int n)
{
  int i;
  int x,y;
  int pos;

  int rows = I->height;
  int cols = I->width;


  for (i = 0 ; i < n ; i++)
  {
    LinearFilter(&laplace,O,I);
    for (y = 0; y < rows; y++)
      for (x = 0; x < cols; x++)
      {
        pos = y * cols + x;
        O->data[pos] = O->data[pos] + dt * I->data[pos];
      }
  }
}

void gaussianFiltrer(Image* I, Image* O, double t)
{
  float pi = M_PI;

  int i, j;
  int u, v;
  int pos;
  int pos_;
  int s;
  int half_size;
  int filter_center;
  double normalization;
  double sum;

  sum = 0.0;
  s = ( 3 * sqrt(t) ) + 1;
  half_size = (2 * s + 1);
  filter_center = (((half_size / 2) + 1) * half_size + ((half_size / 2) + 1));

  Filter gaussian;

  gaussian.size = half_size * half_size;
  gaussian.values = malloc(gaussian.size * sizeof(double));
  normalization = 1 / gaussian.size;

  for (i = 0; i < half_size; i++) for (j = 0; j < half_size; j++)
  {
    pos = i * half_size + j;
    pos_ = pos - filter_center;
    u = i - s;
    v = j - s;
    gaussian.values[pos] =  ( (1 / (4 * pi * t)) * exp(-((u * u + v * v) / (4 * t))));
    sum += gaussian.values[pos];
  }

//Si nous avons une norme proche de 1 il n'est pas nécessaire de normaliser notre filtre

  if (sum < 0.95 || sum > 1.05 )
  {
    for (i = 0 ; i < gaussian.size ; i++)
      gaussian.values[i] = gaussian.values[i] / sum;
  }


  LinearFilter(&gaussian, I, O);
}

double f_de_x (double x, int lambda, int option)
{
  if (option)
    return exp (- (x*x) / (lambda*lambda) );
  else
    return 1 / (1 + ( (x * x) / (lambda * lambda) ) );
}

double gradient (Image* I, int i, int j, int step)
{
  // On a 4 étapes de calcul :
  // Nord, Sud, Est, Ouest
  int rows = I.height;
  int cols = I.width;
  int pos;
  int pos_i_moins;
  int pos_i_plus;
  int pos_j_moins;
  int pos_j_plus;

  pos = i * rows + j;
  pos_j_plus = i * rows + (j+1);
  pos_j_moins = i * rows + (j-1);
  pos_i_plus = (i+1) * rows + j;
  pos_i_moins = (i-1) * rows + j;

  if (i + 1 >= rows )
    pos_i_plus = pos;

  if (i - 1 < 0)
    pos_i_moins = pos;
  
  if (j + 1 >= cols )
    pos_j_plus = pos;

  if (j - 1 < 0)
    pos_j_moins = pos;

  switch (step)
  {
    case 0 :
      return I.data[pos_j_moins] - I.data[pos];
    break;
    
    case 1 :
      return I.data[pos_j_plus] - I.data[pos];
    break;

    case 2 :
      return I.data[pos_i_moins] - I.data[pos];
    break;

    case 3 :
      return I.data[pos_i_plus] - I.data[pos];
    break;
  }
}

void malikAndPerona(Image* I, Image *O, double dt, int n, int lambda, int option)
{
  int i;
  int k;
  double grad;
  Image TMP;
  for (i = 0 ; i < n ; i++)
  {
    //COpier Image o dans tmp
    for (k = 0 ; k < 4 ; k++)
    {
      grad = gradient(TMP,x,y,k);
      O.data[pos] = O.data[pos] + dt * f_de_x(grad, lambda, option)*grad;
    }
  }
}

int main(int argc, char **argv)
{
  LireImage(path, &image);
  LireImage(path, &image_test_in);
  LireImage(path, &image_test_gauss);
  LireImage(path, &image_test_heat);

  initGL();
  glutInit(&argc, argv);

  initLaplaceFilter();

  //Heat(Image *I, Image *O, double dt, double D, int n)
  Heat(&image_test_in, &image_test_heat, 0.25, 5.0, 100);
  //Valeur seuil 0.3 environ

  //gaussianFiltrer(Image *I, Image *O, double t)
  gaussianFiltrer(&image, &image_test_gauss, 20);
  //Ici t remplace n et dt si dt diminue n doit augmenter et si t augment il faut augmenter n ou diminuer dt

  glutInitWindowSize(image.width, image.height);
  glutInitWindowPosition(200, 100);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutCreateWindow("Image filtre chaleur");
  glutDisplayFunc(affichage_h);
  glutReshapeFunc(redim);

  glutInitWindowSize(image.width, image.height);
  glutInitWindowPosition(200, 100);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutCreateWindow("Image filtre Gaussien");
  glutDisplayFunc(affichage_g);
  glutReshapeFunc(redim);

  glutMainLoop();
  return 0;
}