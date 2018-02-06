#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "OutilsPGM.h"

char path[100] = "lena_poivre.pgm";

typedef struct
{
  int size;
  double* values;
} Filter;

Image image;
Image image_test;
Image image_test_in;

GLubyte *I; //Image de travail

Filter laplace;

void initLaplaceFilter()
{
  laplace.size = 9;
  laplace.values = malloc(laplace.size * sizeof(float));

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

void affichage(void)
{
  BasculeImage(&image_test, I);
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

void Heat(Image *I, Image *O, double dt, double D, int n)
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


int main(int argc, char **argv)
{
  LireImage(path, &image);
  LireImage(path, &image_test_in);
  LireImage(path, &image_test);

  initGL();
  glutInit(&argc, argv);

  initLaplaceFilter();

  Heat(&image_test_in, &image_test, 0.25, 5.0, 10);
  //Valeur seuil 0.3 environ

  glutInitWindowSize(image.width, image.height);
  glutInitWindowPosition(200, 100);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutCreateWindow("Image");
  glutDisplayFunc(affichage);
  glutReshapeFunc(redim);

  glutMainLoop();
  return 0;
}