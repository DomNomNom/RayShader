#define GL_GLEXT_PROTOTYPES


#include <stdio.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
// #include <glm/gtc/matrix_projection.hpp>
// #include <glm/gtc/matrix_transform.hpp>

#include "time.h"
#include "shader.h"
#include "math.h"

Shader shader;

GLuint window;
int window_wd = 600;
int window_ht = 600;

float mouse_x = 0.0;
float mouse_y = 0.0;

bool shadeTrace = true;


int numballs = 6;
glm::vec4 ball_pos[] = {  // positions
    glm::vec4(1.0, -.9, 0.0, 1.0),
    glm::vec4(0.0, -.0, 0.3, 1.0),
    glm::vec4(0.5, 0.5, -.3, 1.0),
    glm::vec4(0.3, -.5, -.3, 1.0),
    glm::vec4(-.3, -.5, -.3, 1.0),
    glm::vec4(-.3, 0.5, -.3, 1.0),
};
float ball_radius[] {  // radii
    0.3,
    0.3,
    0.2,
    0.2,
    0.2,
    0.2,
};

float light_direction[] = {1.0f, 0.0f, 0.0f};
GLfloat light_ambient[]     = {0.1, 0.1, 0.1, 1.0};
GLfloat light_diffuse[]     = {0.7, 0.7, 0.7, 1.0};
GLfloat light_specular[]    = {1.0, 1.0, 1.0, 1.0};
GLfloat material_emissive[] = {0.0, 0.0, 0.0, 1.0};
GLfloat material_diffuse[]  = {1.0, 0.0, 0.0, 1.0};
GLfloat material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
GLfloat material_specular[] = {8.8, 8.8, 8.8, 1.0};
GLfloat material_shininess[] = {89};


float extremify(float val) {
    return pow(10*(val-0.5), 5);
}
float openglCoords(float val) {
    return 2.0*val - 1.0;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    ball_pos[0].x = openglCoords(mouse_x);
    ball_pos[0].y = -openglCoords(mouse_y);
    // printf("%f %f\n", balls[0], balls[1]);

    if (shadeTrace) {
        shader.bind();
        // printf("unicorn location: %d\n", glGetUniformLocation(shader.id(), "balls[0].pos"));

        glUniform1i(glGetUniformLocation(shader.id(), "numballs"), numballs);
        glUniform4fv(glGetUniformLocation(shader.id(), "ball_pos"), numballs, &(ball_pos[0].x));
        glUniform1fv(glGetUniformLocation(shader.id(), "ball_radius"), numballs, &(ball_radius[0]));
        // glUniformBlockBinding(shader.id(), glGetUniformLocation(shader.id(), "balls[0].pos"), numballs, &(balls[0].pos.x));
        glUniform2f(glGetUniformLocation(shader.id(), "mouse"), extremify(mouse_x), extremify(mouse_y));

        glColor3f(1,0,0);
        glBegin(GL_TRIANGLES);
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

    glutSwapBuffers();

}


void setupLighting() {
    glPushMatrix();
        glRotatef(60, 0, 0, 1);
        glRotatef(30, 0, 1, 0);
        // glRotatef(90, 0, 1, 0);
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
        case 13:
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
    // glutReshapeFunc();
    // glutIdleFunc();
    // glutMouseFunc();
    glutKeyboardFunc(keyHander);
    glutMotionFunc(mouseMoveHander);
    glutPassiveMotionFunc(mouseMoveHander);


    initShader();
    setupLighting();
    glutMainLoop();

    return 0;
}
