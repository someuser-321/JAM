#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <GLFW/glfw3.h>

#include "noise.c"  // I'll do a Makefile soon enough...


#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define FXAA_SAMPLES 4

//#define PI (3.14159265358979323846)
#define DEG2RAD(x) ((x/180.0f)*PI)
#define DSIN(x) (sin(DEG2RAD(x)))
#define DCOS(x) (cos(DEG2RAD(x)))

#define TIMEDEL60 (1/(double)60)
#define FOV DEG2RAD(60.0f)

#define MOVEMENT_SPEED 2.0f     // units/sec
#define TURN_SPEED 120.0f       // degrees/sec
#define MOUSE_SENSITIVITY 0.05f

#define PERLIN_SIZE 64+2
#define PERLIN_SEED 42
#define PERLIN_OCTAVES 8
#define PERLIN_PERSISTENCE 0.5f

#define MAX_HEIGHT 64.0f


GLFWwindow *window;

double tv0, Tdel;

float rot_x, rot_y;
float pos_x, pos_y, pos_z;

GLuint cubes;
bool cubesCached = false;

bool captureCursor = true;

float movementSpeed = MOVEMENT_SPEED;
float turnSpeed = TURN_SPEED;
bool speedIncreased = false;

float *noiseMap = NULL;
int perlin_seed = PERLIN_SEED;
int perlin_size = PERLIN_SIZE;
float perlin_persistence = PERLIN_PERSISTENCE;

float maxHeight = MAX_HEIGHT;


void setup();
void render_setup();

void step();
void get_input();
void render();

double getFPS();
void cleanup();

void error_callback(int error, const char *description);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void drawCube(int x, int y, int z, float r, float g, float b);


int main(int argc, char *argv[])
{
    int windowWidth = WINDOW_WIDTH;
    int windowHeight = WINDOW_HEIGHT;
    maxHeight = MAX_HEIGHT;
    
    if ( argc >= 3 ) {
        windowWidth = atoi(argv[1]);
        windowHeight = atoi(argv[2]);
    }

    int fxaa_samples = FXAA_SAMPLES;
    if ( argc >= 4 )
        fxaa_samples = atoi(argv[3]);
    
    if ( argc >= 5 )
        perlin_size = atoi(argv[4]) + 2;

    if ( argc >= 6 )
        perlin_persistence = atof(argv[5]);
    
    if ( argc >= 7 )
        maxHeight = atof(argv[6]);
    
    if ( argc >= 8 ) {
        perlin_seed = atoi(argv[7]);
    } else {
        srand((int)time(NULL));
        perlin_seed = rand();
        printf("perlin_seed = %i\n", perlin_seed);
    }

    
    glfwSetErrorCallback(error_callback);

    if ( !glfwInit() )
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_SAMPLES, fxaa_samples);

    window = glfwCreateWindow(windowWidth, windowHeight, "JAM Test", NULL, NULL);
    if ( !window )
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    setup();
    render_setup();
    

    while ( !glfwWindowShouldClose(window) )
    {
        step();
        render();
    }


    cleanup();
    exit(EXIT_SUCCESS);
}

void setup()
{    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    tv0 = glfwGetTime();

    rot_x = 30.0f;
    rot_y = 30.0f;
    pos_x = 0.0f;
    pos_y = 2.0f;
    pos_z = 4.0f;

    noiseMap = fractalNoise(perlin_size, perlin_seed, PERLIN_OCTAVES, perlin_persistence);
}

void render_setup()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = width/(float)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float near = 0.01;
    float far = 1000.0f;

    float top = tanf(FOV*0.5) * near;
    float bottom = -1*top;

    float left = ratio * bottom;
    float right = ratio * top;

    glFrustum(left, right, bottom, top, near, far);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void step()
{
    glfwPollEvents();
    get_input(window);

    char title[64];
    sprintf(title, "JAM Test @ %.1f FPS", (float)getFPS());
    glfwSetWindowTitle(window, title);
}

void get_input()
{
    if ( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if ( glfwGetKey(window, 'W') == GLFW_PRESS ) {
        pos_x += DSIN(rot_y) * movementSpeed * Tdel * DCOS(rot_x);
        pos_z -= DCOS(rot_y) * movementSpeed * Tdel * DCOS(rot_x);
        pos_y -= DSIN(rot_x) * movementSpeed * Tdel;
    }
    if ( glfwGetKey(window, 'S') == GLFW_PRESS ) {
        pos_x -= DSIN(rot_y) * movementSpeed * Tdel * DCOS(rot_x);
        pos_z += DCOS(rot_y) * movementSpeed * Tdel * DCOS(rot_x);
        pos_y += DSIN(rot_x) * movementSpeed * Tdel;
    }
    if ( glfwGetKey(window, 'A') == GLFW_PRESS ) {
        pos_x -= DCOS(rot_y) * movementSpeed * Tdel;
        pos_z -= DSIN(rot_y) * movementSpeed * Tdel;
    }
    if ( glfwGetKey(window, 'D') == GLFW_PRESS ) {
        pos_x += DCOS(rot_y) * movementSpeed * Tdel;
        pos_z += DSIN(rot_y) * movementSpeed * Tdel;
    }
    if ( glfwGetKey(window, 'E') == GLFW_PRESS ) {
        if ( captureCursor ) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            captureCursor = false;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            captureCursor = true;
        }

        for ( int i=0 ; i<31415926 ; i++ ){} // usleep and Sleep aren't working
    }

    if ( glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ) {
        if ( rot_x > -90.0f )
            rot_x -= turnSpeed * Tdel;
    }
    if ( glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ) {
        rot_y -= turnSpeed * Tdel;
    }
    if ( glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ) {
        if ( rot_x < 90.0f )
            rot_x += turnSpeed * Tdel;
    }
    if ( glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ) {
        rot_y += turnSpeed * Tdel;
    }
    if ( glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ) {
        pos_y += movementSpeed * Tdel;
    }
    if ( glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ) {
        pos_y -= movementSpeed * Tdel;
    }
    if ( glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !speedIncreased ) {
        movementSpeed *= 10;
        speedIncreased = true;
    } else if ( glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && speedIncreased ) {
        movementSpeed /= 10;
        speedIncreased = false;
    }
    if ( glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS ) {
        perlin_seed = (int)(glfwGetTime()*1000000);
        printf("seed = %i\n", perlin_seed);
        noiseMap = fractalNoise(perlin_size, perlin_seed, PERLIN_OCTAVES, perlin_persistence);
        cubesCached = false;
        for ( int i=0 ; i<31415926 ; i++ ){}    // usleep and Sleep aren't working
    }
        
    if ( captureCursor ) {
        double x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        rot_y += (x_pos - width/2) * MOUSE_SENSITIVITY;
        float rot_x_ = (y_pos - height/2) * MOUSE_SENSITIVITY;

        if ( rot_x + rot_x_ < 90.0f && rot_x + rot_x_ > -90.0f )
            rot_x += rot_x_;

        glfwSetCursorPos(window, width/2, height/2);
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glPushMatrix();

    glRotatef(rot_x, 1.0f, 0.0f, 0.0f);
    glRotatef(rot_y, 0.0f, 1.0f, 0.0f);
    glTranslatef(-1.0f*pos_x, -1.0f*pos_y, -1.0f*pos_z);

    if ( !cubesCached ) {
        
        cubes = glGenLists(1);
        glNewList(cubes, GL_COMPILE);

        for ( int i=1 ; i<perlin_size-1 ; i++ ) {
            for ( int j=1 ; j<perlin_size-1 ; j++ ) {
                float n = noiseMap[i*perlin_size + j];
                float c = n;
                float lowest = fmin(999.0f, noiseMap[i    *perlin_size + j+1]);
                      lowest = fmin(lowest, noiseMap[i    *perlin_size + j-1]);
                      lowest = fmin(lowest, noiseMap[(i+1)*perlin_size + j  ]);
                      lowest = fmin(lowest, noiseMap[(i-1)*perlin_size + j  ]);

                lowest = floor(lowest*maxHeight);
                int k = (int)floor(n*maxHeight);
                do {
                    drawCube(i-perlin_size/2, k--, j-perlin_size/2, c, c, c);
                } while ( k>lowest );
            }
        }

        glEndList();
        cubesCached = true;
    } else {
        glCallList(cubes);
    }

    glPopMatrix();
    glfwSwapBuffers(window);
}

void cleanup()
{
    free(noiseMap);
    glfwTerminate();
}

void error_callback(int error, const char *description)
{
    fputs(description, stderr);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    render_setup();
}

double getFPS()
{
    double tv1 = glfwGetTime();
    Tdel = tv1 - tv0;
    tv0 = tv1;

    return 1/Tdel;
}

void drawCube(int x, int y, int z, float r, float g, float b)
{
    glPushMatrix();

    glTranslatef(x, y, z);
    glColor3f(r, g, b);
    glPointSize(10.0f);

    glBegin(GL_POINTS);
    
        glVertex3f(0.0f, 0.0f, 0.0f);
    
    glEnd();
    
    glBegin(GL_QUADS);

        glVertex3f( 0.5f, -0.5f, 0.5f);
        glVertex3f( 0.5f,  0.5f, 0.5f);
        glVertex3f(-0.5f,  0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);

        glVertex3f( 0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f,  0.5f, -0.5f);
        glVertex3f( 0.5f,  0.5f,  0.5f);
        glVertex3f( 0.5f, -0.5f,  0.5f);

        glVertex3f(-0.5f, -0.5f,  0.5f);
        glVertex3f(-0.5f,  0.5f,  0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);

        glVertex3f( 0.5f,  0.5f,  0.5f);
        glVertex3f( 0.5f,  0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f,  0.5f);

        glVertex3f( 0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f,  0.5f);
        glVertex3f(-0.5f, -0.5f,  0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);

        glVertex3f( 0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f,  0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);

    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);

        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f,  0.5f);

        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f);

        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f, -0.5f);

        glVertex3f( 0.5f,  0.5f,  0.5f);
        glVertex3f(-0.5f,  0.5f,  0.5f);

        glVertex3f( 0.5f,  0.5f,  0.5f);
        glVertex3f( 0.5f, -0.5f,  0.5f);

        glVertex3f( 0.5f,  0.5f,  0.5f);
        glVertex3f( 0.5f,  0.5f, -0.5f);

        glVertex3f( 0.5f,  0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f, -0.5f);

        glVertex3f( 0.5f, -0.5f,  0.5f);
        glVertex3f(-0.5f, -0.5f,  0.5f);

        glVertex3f( 0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f,  0.5f);

        glVertex3f(-0.5f,  0.5f,  0.5f);
        glVertex3f(-0.5f, -0.5f,  0.5f);

        glVertex3f( 0.5f,  0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f);

        glVertex3f(-0.5f,  0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f,  0.5f);

    glEnd();

    glPopMatrix();
}
