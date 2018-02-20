#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "noise.h"

void gaussianNoise(double var)
{
		//double RANDMAX = 1;
		int i;
		double U = 0.0;
		for (i = 0; i < 12; i++)
		{
			U = (rand() / (double) RAND_MAX);
		printf("%f \n",U);
		}
}

/*void noise(Image* in, Image* out, NoiseType t)
{

}*/