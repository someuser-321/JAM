

#define interp(x1, x2, t) (x1 * (1-t) + x2 * t)
#define scale(x) ((exp(x)-1)/(exp(1)-1))


float *noise(int size, int seed);
float *smoothNoise(float *baseNoise, int size, int octave);
float *fractalNoise(int size, int seed, int octaves, float persistence);
