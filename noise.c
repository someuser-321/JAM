#include <stdlib.h>
#include <math.h>

#include "noise.h"


float *noise(int size, int seed)
{
    srand(seed);
    float *ret = (float*)malloc(size*size*sizeof(float));
    
    for ( int i=0 ; i<size ; i++ ) {
        for ( int j=0 ; j<size ; j++ ) {
            ret[i*size + j] = (float)(rand()%2);
        }
    }
    
    return ret;
}

float *smoothNoise(float *baseNoise, int size, int octave)
{
    float *smoothedNoise = (float*)malloc(size*size*sizeof(float));
    
    int samplePeriod = 1<<octave;
    float sampleFreq = 1.0f/samplePeriod;
    
    for ( int x=0 ; x<size ; x++ )
    {
        int sample_x0 = (x / samplePeriod) * samplePeriod;
        int sample_x1 = (sample_x0 + samplePeriod) % size;
        float horizontalBlend = (x - sample_x0) * sampleFreq;
        
        for ( int y=0 ; y<size ; y++ )
        {
            int sample_y0 = (y / samplePeriod) * samplePeriod;
            int sample_y1 = (sample_y0 + samplePeriod) % size;
            float verticalBlend = (y - sample_y0) * sampleFreq;
            
            float top = interp(baseNoise[sample_x0*size + sample_y0],
                               baseNoise[sample_x1*size + sample_y0], horizontalBlend);
            float bottom = interp(baseNoise[sample_x0*size + sample_y1],
                                  baseNoise[sample_x1*size + sample_y1], horizontalBlend);
            
            smoothedNoise[x*size + y] = interp(top, bottom, verticalBlend);
        }
    }
    
    return smoothedNoise;
}

float *fractalNoise(int size, int seed, int octaves, float persistence)
{
    
    float *fractalNoise = (float*)malloc(size*size*sizeof(float));
    
    for ( int x=0 ; x<size ; x++ ) {
        for ( int y=0 ; y<size ; y++ ) {
            fractalNoise[x*size + y] = 0.0f;
        }
    }
    
    float amplitude = 1.0f;
    float totalAmplitude = 0.0f;
    float *baseNoise = noise(size, seed);
    
    for ( int octave=octaves-1 ; octave>=0 ; octave-- )
    {
        amplitude *= persistence;
        totalAmplitude += amplitude;
        
        float *tempNoise = smoothNoise(baseNoise, size, octave);
        
        for ( int x=0 ; x<size ; x++ ) {
            for ( int y=0 ; y<size ; y++ ) {
                fractalNoise[x*size + y] += tempNoise[x*size + y] * amplitude;
            }
        }
        
        free(tempNoise);
    }
    
    free(baseNoise);
    
    float minValue = fractalNoise[0];
    float maxValue = fractalNoise[0];
    
    for ( int x=0 ; x<size ; x++ ) {
        for ( int y=0 ; y<size ; y++ ) {
            fractalNoise[x*size + y] /= totalAmplitude;
            minValue = fmin(minValue, fractalNoise[x*size + y]);
            maxValue = fmax(maxValue, fractalNoise[x*size + y]);
        }
    }
    
    for ( int x=0 ; x<size ; x++ ) {
        for ( int y=0 ; y<size ; y++ ) {
            fractalNoise[x*size + y] = scale(((fractalNoise[x*size + y]) - minValue) / (maxValue - minValue));
        }
    }
    
    return fractalNoise;
}