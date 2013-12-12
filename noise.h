#include <math.h>

#define PI 3.1415926535f

#define interp(x1, x2, t) (x1*(1-(1-cos(t*PI))/2) + x2*(1-cos(t*PI))/2)
#define scale(x) ((exp(x)-1)/(exp(1)-1))


float *noise(int size, int seed);
float *smoothNoise(float *baseNoise, int size, int octave);
float *fractalNoise(int size, int seed, int octaves, float persistence);
