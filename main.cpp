#define GL_GLEXT_PROTOTYPES


#include <stdio.h>
#include <GL/glut.h>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/matrix_projection.hpp>

#include "time.h"
#include "shader.h"
#include "textures.h"
#include "scene.h"


using namespace glm;


#define F 1000.0 // floor size

Shader shader;

GLuint window;
int window_wd = 600;
int window_ht = 600;
float aspectRatio = window_wd / window_ht;

float mouse_x = 0.5;
float mouse_y = 0.5;
float seconds = 0;

bool shadeTrace = true;

float PI = acos(0.0) * 2.0;

float modelScale = 0.7;




scene currentScene = SCENE_WATER;
std::vector<vec4> vertecies;
std::vector<int> triangles;
std::vector<vec4> ball_pos;
std::vector<float> ball_radius;
GLuint skybox;



// modified by water simulation
std::vector<vec2> water;
int trbulentuMin, trbulentuMax;


// lights
float light_direction[] = {1.0f, 0.0f, 0.0f};
GLfloat light_ambient[]     = {0.1, 0.1, 0.1, 1.0};
GLfloat light_diffuse[]     = {0.7, 0.7, 0.7, 1.0};
GLfloat light_specular[]    = {1.0, 1.0, 1.0, 1.0};
GLfloat material_emissive[] = {0.0, 0.0, 0.0, 1.0};
GLfloat material_diffuse[]  = {1.0, 0.0, 0.0, 1.0};
GLfloat material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
GLfloat material_specular[] = {8.8, 8.8, 8.8, 1.0};
GLfloat material_shininess[] = {89};


mat4 cameraTransform = mat4(1.0f);
// mat4 view = mat4(1.0f);
// void applyView(mat4 viewMatrix) {
//     view = viewMatrix;
//     for (unsigned int i=0; i<ball_pos.size(); ++i) {
//         ball_pos[i] = view * ball_pos[i];
//     }
//     for (unsigned int i=0; i<vertecies.size(); ++i) {
//         vertecies[i] = view * vertecies[i];
//     }
// }
// void undoView() {
//     applyView(inverse(view));
// }

void printVec(vec4 *v){
    printf(
        "[%f %f %f]\n",
        v->x,
        v->y,
        v->z
    );
}
void myTranslate(vec4 v) {
    glTranslatef(
        v.x,
        v.y,
        v.z
    );
}


float extremify(float val) {
    return pow(10*(val-0.5), 5);
}
float openglCoords(float val) {
    return 2.0*val - 1.0;
}

void display() {
    seconds += time_dt();
    // printf("time %f\n", seconds);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // printf("%f\n", modelScale);
    // ball_pos[0].x = openglCoords(mouse_x);
    // ball_pos[0].y = -openglCoords(mouse_y);
    // printf("%f %f\n", balls[0], balls[1]);

    // float minY = 0.0f;
    // for (unsigned int i=0; i<ball_radius.size(); ++i) {
    //     // ball_radius[i] = mouse_y;
    //     ball_radius[i] = 0.15;

    //     // set the floor to be the lowest possible point
    //     minY = min(minY, ball_pos[i].y - ball_radius[i]);
    // }
    // for (int i=1*3; i<3*3; i+=3) {
    //     triangles[i].y = minY;
    // }
    // triangles[8].y = 40 * openglCoords( mouse_y);


    // how the camera is transformed
    cameraTransform = mat4(1.0f);
    cameraTransform = rotate(cameraTransform, mouse_x * 500.0f, vec3(0.0f, 1.0f, 0.0f));
    cameraTransform = rotate(cameraTransform, mouse_y * 180.0f-90.0f, vec3(1.0f, 0.0f, 0.0f));
    cameraTransform = translate(cameraTransform, vec3(0.0, 0.0, -2.0));


    // modelScale = mouse_x+0.1f;
    // modelScale = 0.7f;
    // mat4 transform = mat4(1.0f);
    // transform = scale(transform, vec3(modelScale, modelScale, modelScale));
    // transform = rotate(transform, mouse_x * 500.0f, vec3(0.0f, 1.0f, 0.0f));
    // transform = rotate(transform, seconds * 50.0f, vec3(0.0f, 1.0f, 0.0f));
    // applyView(transform);
    // printVec(triangles+1);

    // triangles[0].x = openglCoords(mouse_x);

    if (shadeTrace) {

        // pass texture samplers
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
        // glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        shader.bind();

        // pass the data to the shader
        // glUniform4fv(glGetUniformLocation(shader.id(), "triangles"),    numTriangles*3, value_ptr(triangles[0]) );
        glUniform4fv(glGetUniformLocation(shader.id(), "vertecies"),    vertecies.size(), value_ptr(vertecies[0]) );
        glUniform1iv(glGetUniformLocation(shader.id(), "triangles"),    triangles.size(), &triangles[0] );
        glUniform1i( glGetUniformLocation(shader.id(), "numTriangles"), triangles.size()/3);
        glUniform1i( glGetUniformLocation(shader.id(), "numBalls"),    ball_pos.size());
        glUniform4fv(glGetUniformLocation(shader.id(), "ball_pos"),    ball_pos.size(), value_ptr(ball_pos[0]) );
        glUniform1fv(glGetUniformLocation(shader.id(), "ball_radius"), ball_pos.size(), &ball_radius[0]);
        glUniform2f( glGetUniformLocation(shader.id(), "mouse"), extremify(mouse_x), extremify(mouse_y));
        glUniform1i( glGetUniformLocation(shader.id(), "skybox"), 0); //Texture unit 0
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "cameraTransform"), 1, false, value_ptr(cameraTransform));
        glUniform1f( glGetUniformLocation(shader.id(), "time"), seconds); //Texture unit 0

        float r = 10.0;
        glColor3f(1,0,0);
        glBegin(GL_TRIANGLES);
            // two triangles that cover the screen
            glVertex3f(-r,-r, 0.0);
            glVertex3f( r,-r, 0.0);
            glVertex3f(-r, r, 0.0);

            glVertex3f( r, r, 0.0);
            glVertex3f( r,-r, 0.0);
            glVertex3f(-r, r, 0.0);
        glEnd();

        shader.unbind();
    }
    else { // openGL render
        glEnable(GL_LIGHTING);

            for (unsigned int i=0; i<ball_pos.size(); ++i) {
                glPushMatrix();
                    myTranslate(ball_pos[i]);
                    glutSolidSphere(ball_radius[i], 32, 32);
                glPopMatrix();
            }

            glBegin(GL_TRIANGLES);
            for (unsigned int i=0; i<triangles.size(); i+=3) {
                glVertex3fv(value_ptr(vertecies[triangles[i  ]]));
                glVertex3fv(value_ptr(vertecies[triangles[i+1]]));
                glVertex3fv(value_ptr(vertecies[triangles[i+2]]));
            }
            glEnd();

        glDisable(GL_LIGHTING);
    }

    // undoView();

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

void reloadScene() {
    loadScene(
        currentScene,
        vertecies,
        triangles,
        ball_pos,
        ball_radius
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
        case '1': currentScene = SCENE_BEACH;      reloadScene(); break;
        case '2': currentScene = SCENE_SURFACE;    reloadScene(); break;
        case '3': currentScene = SCENE_WATER;      reloadScene(); break;
    }
    glutPostRedisplay();
}


void reshapeHandler(int, int ht) {
    glViewport(0, 0, ht/aspectRatio, ht);
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
    // glutIdleFunc(display);
    glutReshapeFunc(reshapeHandler);
    // glutMouseFunc();
    glutKeyboardFunc(keyHander);
    glutMotionFunc(mouseMoveHander);
    glutPassiveMotionFunc(mouseMoveHander);

    // // initialize triangles
    // for (int i=0; i<numTriangles; ++i) {
    //     float theta = i/float(numTriangles) * 2.0*PI;
    //     triangles[i*3 +2].y = cos(theta);
    //     triangles[i*3 +2].z = sin(theta);
    // printf("A: %f %f\n", triangles[i*3 +2].y, triangles[i*3 +2].z);
    // }


    // skybox = png_texture_load("sky.png", &skybox_wd, &skybox_ht);
    reloadScene();
    skybox = png_cubemap_load("resources/beach/");
    initShader();
    setupLighting();
    glutMainLoop();

    return 0;
}
