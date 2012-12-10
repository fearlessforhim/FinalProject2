//
//  main.c
//  SierpinskiTriangleGLUT
//
//  Created by Jonathan Cameron on 9/12/12.
//  Copyright (c) 2012 Jonathan Cameron. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>
#include <time.h>

#include "camera.h"

static void display() {
    
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);

    float vert1[3] = {0.0, 0.0, 0.0};
    float vert2[3] = {1.0, 0.0, 0.0};
    float vert3[3] = {0.0, 1.0, 0.0};
    
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3fv(vert1);
    glVertex3fv(vert2);
    glVertex3fv(vert3);
    glEnd();
    glFlush();
}

static void draw_verts(){
    
}

static void set_projection(){
    camera_lookat();
}

static void set_perspective(){
    gluPerspective(70, 1.0, 0.1, 10);
}

int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
    glutCreateWindow("Music Bars");
    glutReshapeWindow(glutGet(GLUT_SCREEN_WIDTH)/1.5, glutGet(GLUT_SCREEN_HEIGHT)/1.5);
    glutDisplayFunc(display);
    set_projection();
    glutMainLoop();
}

