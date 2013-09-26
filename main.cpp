#define GL_GLEXT_PROTOTYPES


#include <stdio.h>
#include <GL/glut.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/matrix_projection.hpp>

#include "time.h"
#include "shader.h"
#include "textures.h"
#include "math.h"


#define F 10.0 // floor size

Shader shader;

GLuint window;
int window_wd = 600;
int window_ht = 600;

float mouse_x = 0.5;
float mouse_y = 0.25;
float millis = 0;

bool shadeTrace = true;


int numTriangles = 1;
glm::vec4 triangles[] = {
    //  point A,                             B-A,                            C-A
    glm::vec4(0.0, 0.0,  0.0, 1.0), glm::vec4( 0.5, 0.5, 0.0, 0.0), glm::vec4(-0.25, 0.5,  0.25, 0.0),
    glm::vec4( -F, -.5,   -F, 1.0), glm::vec4( 2*F, 0.0, 0.0, 0.0), glm::vec4( 0.0, 0.0,  2*F, 0.0),
    glm::vec4(  F, -.5,    F, 1.0), glm::vec4(-2*F, 0.0, 0.0, 0.0), glm::vec4( 0.0, 0.0, -2*F, 0.0),
};

// balls.
// TODO move this to a external file
int numballs = 12;
glm::vec4 ball_pos[] = {  // positions
    glm::vec4(0,                 -0.525731,          0.850651,  1.0),
    glm::vec4(0.850651,           0,                 0.525731,  1.0),
    glm::vec4(0.850651,           0,                -0.525731,  1.0),
    glm::vec4(-0.850651,          0,                -0.525731,  1.0),
    glm::vec4(-0.850651,          0,                 0.525731,  1.0),
    glm::vec4(-0.525731,          0.850651,          0       ,  1.0),
    glm::vec4(0.525731,           0.850651,          0       ,  1.0),
    glm::vec4(0.525731,          -0.850651,          0       ,  1.0),
    glm::vec4(-0.525731,         -0.850651,          0       ,  1.0),
    glm::vec4(0,                 -0.525731,         -0.850651,  1.0),
    glm::vec4(0,                  0.525731,         -0.850651,  1.0),
    glm::vec4(0,                  0.525731,          0.850651,  1.0),
};
float ball_radius[] = {  // radii
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
    0.25,
};
float modelScale = 0.7;

GLuint skybox;
// int skybox_wd, skybox_ht;


float light_direction[] = {1.0f, 0.0f, 0.0f};
GLfloat light_ambient[]     = {0.1, 0.1, 0.1, 1.0};
GLfloat light_diffuse[]     = {0.7, 0.7, 0.7, 1.0};
GLfloat light_specular[]    = {1.0, 1.0, 1.0, 1.0};
GLfloat material_emissive[] = {0.0, 0.0, 0.0, 1.0};
GLfloat material_diffuse[]  = {1.0, 0.0, 0.0, 1.0};
GLfloat material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
GLfloat material_specular[] = {8.8, 8.8, 8.8, 1.0};
GLfloat material_shininess[] = {89};


glm::mat4 view = glm::mat4(1.0f);
void applyView(glm::mat4 viewMatrix) {
    view = viewMatrix;
    for (int i=0; i<numballs; ++i) {
        ball_pos[i] = view * ball_pos[i];
    }
    for (int i=0; i<numTriangles*3; i+=3) {
        triangles[i  ] = view * triangles[i  ];
        triangles[i+1] = view * triangles[i+1];
        triangles[i+2] = view * triangles[i+2];
    }
}
void undoView() {
    applyView(glm::inverse(view));
}

void printVec(glm::vec4 *v){
    printf(
        "[%f %f %f]\n",
        v->x,
        v->y,
        v->z
    );
}


float extremify(float val) {
    return pow(10*(val-0.5), 5);
}
float openglCoords(float val) {
    return 2.0*val - 1.0;
}

void display() {
    millis += time_dt() * 1000;
    // printf("time %f\n", millis);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // printf("%f\n", modelScale);
    // ball_pos[0].x = openglCoords(mouse_x);
    // ball_pos[0].y = -openglCoords(mouse_y);
    // printf("%f %f\n", balls[0], balls[1]);

    float minY = 0.0f;
    for (int i=0; i<numballs; ++i) {
        ball_radius[i] = mouse_y;
        // ball_radius[i] = 0.01;
        minY = glm::min(minY, ball_pos[i].y - ball_radius[i]*2);
    }

    // set the floor to be the lowest possible point
    for (int i=1*3; i<3*3; i+=3) {
        triangles[i].y = minY;
    }
    // triangles[8].y = 40 * openglCoords( mouse_y);


    glm::mat4 transform = glm::mat4(1.0f);

    modelScale = mouse_x+0.1f;
    modelScale = 0.7f;
    transform = glm::scale(transform, glm::vec3(modelScale, modelScale, modelScale));
    transform = glm::rotate(transform, mouse_x * 500.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    // transform = glm::rotate(transform, millis * 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
    applyView(transform);
    // printVec(triangles+1);

    // triangles[0].x = openglCoords(mouse_x);

    if (shadeTrace) {

        // pass texture samplers
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        shader.bind();

        // pass the data to the shader
        glUniform1i( glGetUniformLocation(shader.id(), "numTriangles"),    numTriangles);
        glUniform4fv(glGetUniformLocation(shader.id(), "triangles"),    numTriangles*3, &(triangles[0].x) );
        glUniform1i( glGetUniformLocation(shader.id(), "numballs"),    numballs);
        glUniform4fv(glGetUniformLocation(shader.id(), "ball_pos"),    numballs, &(ball_pos[0].x) );
        glUniform1fv(glGetUniformLocation(shader.id(), "ball_radius"), numballs, &(ball_radius[0]));
        glUniform2f( glGetUniformLocation(shader.id(), "mouse"), extremify(mouse_x), extremify(mouse_y));
        glUniform1i( glGetUniformLocation(shader.id(), "skybox"), 0); //Texture unit 0


        glColor3f(1,0,0);
        glBegin(GL_TRIANGLES);
            // two triangles that cover the screen
            glVertex3f(-1,-1, 0.0);
            glVertex3f( 1,-1, 0.0);
            glVertex3f(-1, 1, 0.0);

            glVertex3f( 1, 1, 0.0);
            glVertex3f( 1,-1, 0.0);
            glVertex3f(-1, 1, 0.0);
        glEnd();

        shader.unbind();
    }
    else {
        glEnable(GL_LIGHTING);

            for (int i=0; i<numballs; ++i) {
                glPushMatrix();
                    glTranslatef(
                        ball_pos[i].x,
                        ball_pos[i].y,
                        ball_pos[i].z
                    );
                    glutSolidSphere(ball_radius[i], 32, 32);
                glPopMatrix();
            }

        glDisable(GL_LIGHTING);
    }

    undoView();

    glutSwapBuffers();

}


void setupLighting() {
    glPushMatrix();
        glRotatef(60, 0, 0, 1);
        glRotatef(30, 0, 1, 0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_direction);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glEnable(GL_LIGHT0);
    glPopMatrix();
}



void initShader() {
    shader.init(
        "shaders/vert.glsl",
        "shaders/frag.glsl"
    );
}

void keyHander(unsigned char key, int, int) {
    switch (key) {
        case 27: // Escape -> exit
            glutDestroyWindow(window);
            exit(0);
            break;
        case 13:  // Enter -> refresh shader
            initShader();
            break;
        case 's':
            shadeTrace = !shadeTrace;
            break;

    }
    glutPostRedisplay();
}
void mouseMoveHander(int x, int y){
    mouse_x = x/(float)window_wd;
    mouse_y = y/(float)window_ht;
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    time_init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    window = glutCreateWindow("RayShader");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    // glutReshapeFunc();
    // glutMouseFunc();
    glutKeyboardFunc(keyHander);
    glutMotionFunc(mouseMoveHander);
    glutPassiveMotionFunc(mouseMoveHander);



    // skybox = png_texture_load("sky.png", &skybox_wd, &skybox_ht);
    skybox = png_cubemap_load("resources/beach/");
    initShader();
    setupLighting();
    glutMainLoop();

    return 0;
}
