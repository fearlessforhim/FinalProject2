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
#include <string.h>

#include "camera.h"
#include "precision.h"

//object arrays
static Real *vertices;
static int *faces;
static int numOfVerts = 0;
static int numOfFaces = 0;

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
    
    camera_lookat();
}

static void idle(){
    glutPostRedisplay();
}

static void draw_verts(){
    
}

static void set_projection(){
    camera_lookat();
}

static void keyboard(unsigned char key, int x, int y) {
    
	if(camera_keyboard(key)) {
		return;
	}
	switch(key) {
//        case KEY_TERRAIN_DRAW:
//            draw_terrain = !draw_terrain;
//            printf("draw_terrain is %d\n", draw_terrain);
//            fflush(stdout);
//            break;
//        case KEY_WIRE:
//            if(shift_key_down) {
//                wireframe = !wireframe;
//                printf("wireframe is %d\n", wireframe);
//                grid_terrain_set_draw_op(wireframe?GL_LINE_LOOP:GL_TRIANGLES);
//            }
//            break;
//        case KEY_ESC:
//            exit(0);
        default:
            break;
	}
}

static void set_perspective(){
    gluPerspective(70, 1.0, 0.1, 10);
}


static void parse_obj(char* fileName){
	FILE* obj_file_stream;
	char read_line[500];
	char current_line[500];
	char current_line2[500];
	char *current_token;
	char verts[4];
	Real dVar;
	int iVar;
	int vert_count = 0;
	int face_count = 0;
	
	printf("Parsing file\n");
	
	obj_file_stream = fopen( fileName, "r");
	if(obj_file_stream == 0){
		printf("Error reading file\n");
	}
	
	while( fgets(read_line, 500, obj_file_stream) ){
        
		current_token = strtok(read_line, " ");
		printf("token: %s\n", current_token);
		
		if( current_token == NULL || current_token[0] == '#'){
			printf("Skipping\n");
			continue;
		}//parse objects
		else if(current_token[0] == 'v') //process vertex
		{
			current_token = strtok(NULL, " ");
			while(current_token != NULL){
				dVar = atof(current_token);
				vertices[vert_count++] = dVar;
				current_token = strtok(NULL, " ");
			}
		}else if( current_token[0] == 'f') //process face
		{
			current_token = strtok(NULL, " ");
			while(current_token != NULL){
				iVar = atoi(current_token);
				faces[face_count++] = iVar;
				current_token = strtok(NULL, " ");
			}
		}
	}
}

static void getNumOfVertsFaces(char* fileName){
	FILE* obj_file_stream;
	char read_line[500];
	char *current_token;
	Real dVar;
	int iVar;
	int vert_count = 0;
	int face_count = 0;
	
	printf("Parsing file\n");
	
	obj_file_stream = fopen( fileName, "r");
	if(obj_file_stream == 0){
		printf("Error reading file\n");
	}
	
	while( fgets(read_line, 500, obj_file_stream) ){
        
		current_token = strtok(read_line, " ");
		printf("token: %s\n", current_token);
		
		if( current_token == NULL || current_token[0] == '#'){
			printf("Skipping\n");
			continue;
		}//parse objects
		else if(current_token[0] == 'v') //process vertex
		{
			current_token = strtok(NULL, " ");
			while(current_token != NULL){
				numOfVerts++;
				current_token = strtok(NULL, " ");
			}
		}else if( current_token[0] == 'f') //process face
		{
			current_token = strtok(NULL, " ");
			while(current_token != NULL){
				numOfFaces++;
				current_token = strtok(NULL, " ");
			}
		}
	}
}

static void init(){
    getNumOfVertsFaces("box.obj");
    
    vertices = (Real*)malloc(numOfVerts*sizeof(Real));
    faces = (int*)malloc(numOfFaces*sizeof(int));
    parse_obj("box.obj");
}


int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
    glutCreateWindow("Music Bars");
    glutReshapeWindow(glutGet(GLUT_SCREEN_WIDTH)/1.5, glutGet(GLUT_SCREEN_HEIGHT)/1.5);
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    
    init();
    
    set_projection();
    glutMainLoop();
}

