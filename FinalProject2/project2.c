////
////  main.c
////  SierpinskiTriangleGLUT
////
////  Created by Jonathan Cameron on 9/12/12.
////  Copyright (c) 2012 Jonathan Cameron. All rights reserved.
////
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>

#include "precision.h"
#include "geom.h"
#include "camera.h"

//#include "mvm.h"

#define FPS 60
#define KEY_ESC 27
#define KEY_RESET_TRANSFORM 'r'
#define KEY_TOGGLE_LOCAL_TRANSFORM ' '

// window
static const char* win_title = "Music Bars";
static int win_id = 0;
static int init_win_width = 800;
static int init_win_height = 600;
static int win_w() {return glutGet(GLUT_WINDOW_WIDTH);}
static int win_h() {return glutGet(GLUT_WINDOW_HEIGHT);}

// tick rate
static double frame_time = 1.0 / FPS;
static double prev_tick_sec = 0;

// fps
static int frame_count = 0;
static double prev_fps_sec = 0;
static int prev_fps_frame = 0;
static double fps_update_interval = 1.0;

//object arrays
static Real **vertices;
static int **faces;
static int *numOfVerts;
static int *numOfFaces;
static int objectVertNum;
static int objectFaceNum;
static int numOfObjects = 2;

//drawing
Real norm[3];
Real vert1[3];
Real vert2[3];
Real vert3[3];
Real center[3];

// ===== util

// returns current time in seconds
static double get_time() {
	struct timeval now;
	gettimeofday(&now, NULL);
	double now_sec = now.tv_sec + now.tv_usec * 1e-6;
	return now_sec;
}

// ===== input
static void keyboard(unsigned char key, int x, int y) {
    
    if(camera_keyboard(key))
        return;
    
	switch(key) {
        case KEY_ESC:
            exit(0);
            
        case KEY_RESET_TRANSFORM:
            break;
            
        case KEY_TOGGLE_LOCAL_TRANSFORM:
            break;
            
        default:
            break;
	}
}

static void special(int key, int x, int y) {
	int translate = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
    
	float dir;
    
	switch(key) {
        case KEY_ESC:
            exit(0);
            
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:
            dir = key == GLUT_KEY_UP ? 1 : -1;
            if(translate)

            break;
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            dir = key == GLUT_KEY_RIGHT ? 1 : -1;
            if(translate)
            break;
            
        default:
            break;
	}
}

// ===== drawing

static void draw_new_tri(Real v1[3], Real v2[3], Real v3[3]){
        
    glBegin(GL_TRIANGLES);
    geom_calc_normal(v1, v2, v3, norm);
    glNormal3rv(norm);
    glVertex3rv(v1);
    glVertex3rv(v2);
    glVertex3rv(v3);
    glEnd();
}

static void draw_verts(Real c[3], int objNum){

    
    printf("SIZE OF vertices: %lu", sizeof(vertices[objNum]));
    
    for(int x = 0; x<numOfFaces[objNum]; x+=3){
        vert1[0] = vertices[objNum][((faces[objNum][x]-1)*3)];
        vert1[1] = vertices[objNum][((faces[objNum][x]-1)*3)+1];
        vert1[2] = vertices[objNum][((faces[objNum][x]-1)*3)+2] + c[2];
        
        vert2[0] = vertices[objNum][((faces[objNum][x+1]-1)*3)];
        vert2[1] = vertices[objNum][((faces[objNum][x+1]-1)*3)+1];
        vert2[2] = vertices[objNum][((faces[objNum][x+1]-1)*3)+2] + c[2];
        
        vert3[0] = vertices[objNum][((faces[objNum][x+2]-1)*3)];
        vert3[1] = vertices[objNum][((faces[objNum][x+2]-1)*3)+1];
        vert3[2] = vertices[objNum][((faces[objNum][x+2]-1)*3)+2] + c[2];
        
        draw_new_tri(vert1, vert2, vert3);
    }
}

static void display() {
    
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	// set camera viewpoint
	camera_lookat();

    center[0] = 0.0;
    center[1] = 0.0;
    center[2] = 0.0;
    
    for(int objNum = 0; objNum < numOfObjects; objNum++){
        draw_verts(center, objNum);
        center[2] = -2.0;
    }
        
	glFlush();
    
	// increment frame count for framerate code
	++frame_count;
}

static void set_projection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float fov = 90;
	float aspect = win_w()/win_h();
	float n = 0.1;
	float f = 100;
	gluPerspective(fov, aspect, n, f);
	glMatrixMode(GL_MODELVIEW);
}

static void set_lighting() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
}

static void idle() {
	// tick sim every frame
	// call game tick every frame interval
	double now_sec = get_time();
	double diff_tick_sec = now_sec - prev_tick_sec;
	if(diff_tick_sec > frame_time) {
		prev_tick_sec = now_sec;
		glutPostRedisplay();
	}
    
	// calc fps every fps update interval
	double diff_fps_sec = now_sec - prev_fps_sec;
	if(diff_fps_sec > fps_update_interval) {
		int diff_frames = frame_count - prev_fps_frame;
		double fps = diff_frames / diff_fps_sec;
		char title[256];
		sprintf(title, "%s - %d fps", win_title, (int)fps);
		glutSetWindowTitle(title);
		glutPostRedisplay();
		prev_fps_sec = now_sec;
		prev_fps_frame = frame_count;
	}
}

static void parse_obj(char* fileName, int objNum){
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
				dVar = atof(current_token);
				vertices[objNum][vert_count++] = dVar;
				current_token = strtok(NULL, " ");
			}
		}else if( current_token[0] == 'f') //process face
		{
			current_token = strtok(NULL, " ");
			while(current_token != NULL){
				iVar = atoi(current_token);
				faces[objNum][face_count++] = iVar;
				current_token = strtok(NULL, " ");
			}
		}
	}
}

static void getNumOfVertsFaces(char* fileName, int objNum){
	FILE* obj_file_stream;
	char read_line[500];
	char *current_token;

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
				objectVertNum++;
				current_token = strtok(NULL, " ");
			}
		}else if( current_token[0] == 'f') //process face
		{
			current_token = strtok(NULL, " ");
			while(current_token != NULL){
				objectFaceNum++;
				current_token = strtok(NULL, " ");
			}
		}
	}
    
    vertices[objNum] = (Real*)malloc(objectVertNum*sizeof(Real));
    faces[objNum] = (int*)malloc(objectFaceNum*sizeof(int));
    
    numOfFaces[objNum] = objectFaceNum;
    numOfVerts[objNum] = objectVertNum;
}


static void init(){
    
    vertices = (Real**)malloc(numOfObjects*sizeof(Real*));//create multidimensinal arrays
    faces = (int**)malloc(numOfObjects*sizeof(int*));
    numOfVerts = (int*)malloc(numOfObjects*sizeof(int));
    numOfFaces = (int*)malloc(numOfObjects*sizeof(int));
    
    getNumOfVertsFaces("box.obj", 0);//get number of verts and faces for box.obj
    getNumOfVertsFaces("sphere.obj", 1);

    parse_obj("box.obj", 0);
    parse_obj("sphere.obj", 1);
}

int main(int argc, char* argv[]) {
    //glut initialization
	glutInit(&argc, argv);
    
	glutInitWindowSize(init_win_width, init_win_height);
	win_id = glutCreateWindow(win_title);
	if(win_id <= 0) {
		fprintf(stderr, "Error: glutCreateWindow() returned %d\n", win_id);
		exit(1);
	}
    
    
    //scene initialization
    init();
    
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutIdleFunc(idle);
    
	set_projection();
	set_lighting();
	
	glutMainLoop();
    
	return 0;
}

