////
////  main.c
////  SierpinskiTriangleGLUT
////
////  Created by Jonathan Cameron on 9/12/12.
////  Copyright (c) 2012 Jonathan Cameron. All rights reserved.
////
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <OpenGL/OpenGL.h>
//#include <GLUT/GLUT.h>
//#include <time.h>
//#include <string.h>
//
//#include "camera.h"
//#include "precision.h"
//

//
//static void display() {
//    
//    glLoadIdentity();
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    
//    camera_lookat();
//
//    float vert1[3] = {0.0, 0.0, 0.5};
//    float vert2[3] = {1.0, 0.0, 0.5};
//    float vert3[3] = {0.0, 1.0, 0.5};
//    
//    glBegin(GL_TRIANGLES);
//    glColor3f(1.0, 1.0, 0.0);
//    glVertex3fv(vert1);
//    glVertex3fv(vert2);
//    glVertex3fv(vert3);
//    glEnd();
//    
////    vert1[0] = 0;
////    vert1[1] = 0;
////    vert1[2] = -1;
////    glBegin(GL_TRIANGLES);
////    glColor3f(1.0, 1.0, 0.0);
////    glVertex3fv(vert1);
////    glVertex3fv(vert2);
////    glVertex3fv(vert3);
////    glEnd();
//    
//    
//    glFlush();
//}
//
//static void idle(){
//    glutPostRedisplay();
//}
//
//static void draw_obj_verts(){
//    
//}
//
//
//
//static void keyboard(unsigned char key, int x, int y) {
//    
//	if(camera_keyboard(key)) {
//		return;
//	}
//	switch(key) {
////        case KEY_TERRAIN_DRAW:
////            draw_terrain = !draw_terrain;
////            printf("draw_terrain is %d\n", draw_terrain);
////            fflush(stdout);
////            break;
////        case KEY_WIRE:
////            if(shift_key_down) {
////                wireframe = !wireframe;
////                printf("wireframe is %d\n", wireframe);
////                grid_terrain_set_draw_op(wireframe?GL_LINE_LOOP:GL_TRIANGLES);
////            }
////            break;
////        case KEY_ESC:
////            exit(0);
//        default:
//            break;
//	}
//}
//
//static void set_projection(){
//    camera_lookat();
//}
//
//static void set_perspective(){
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    gluPerspective(70, 1.0, 0.1, 10.0);
//    glMatrixMode(GL_MODELVIEW);
//}
//
//static void set_lighting() {
//	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//	glEnable(GL_COLOR_MATERIAL);
//	glEnable(GL_NORMALIZE);
//}
//
//


//
//
//int main(int argc, char * argv[])
//{
//    glutInit(&argc, argv);
//    glutCreateWindow("Music Bars");
//    glutReshapeWindow(glutGet(GLUT_SCREEN_WIDTH)/1.5, glutGet(GLUT_SCREEN_HEIGHT)/1.5);
//    
//    glutDisplayFunc(display);
//    glutKeyboardFunc(keyboard);
//    glutIdleFunc(idle);
//    
//    init();
//    
//    set_projection();
//    set_lighting();
//    glutMainLoop();
//}
//


// manip_teapot.c	: keyboard-driven manipulation of teapot position and orientation in GLUT
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

struct Frame {
	Real basis[3*3];
	Real origin[3];
};
typedef struct Frame Frame;

// window
static const char* win_title = "manip_teapot";
static int win_id = 0;
static int init_win_width = 800;
static int init_win_height = 600;
static int win_w() {return glutGet(GLUT_WINDOW_WIDTH);}
static int win_h() {return glutGet(GLUT_WINDOW_HEIGHT);}

// input
static Real rot_angle = 5.0;
static Real trans_dist = 0.1;

// tick rate
static double frame_time = 1.0 / FPS;
static double prev_tick_sec = 0;

// fps
static int frame_count = 0;
static double prev_fps_sec = 0;
static int prev_fps_frame = 0;
static double fps_update_interval = 1.0;

// teapot
static Real teapot_color[3] = {0.8, 0.5, 0.4};
Frame teapot_frame = {
	{
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
	},
	{ 0, 0, 0},
};
int local_transform = 0;

//object arrays
static Real *vertices;
static int *faces;
static int numOfVerts = 0;
static int numOfFaces = 0;


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
            local_transform = !local_transform;
            printf("local_transform: %d\n", local_transform);
            break;
            
        default:
            break;
	}
}

static void translate_x(float dist) {
	Frame* frm = &teapot_frame;
	Real vtrans[3] = {dist, 0, 0};
	if(local_transform) {
		// transform vector by current rotation matrix
		geom_vector3_matrix3_mul(vtrans, frm->basis, vtrans);
	}
	geom_vector3_add(frm->origin, vtrans, frm->origin);
}

static void translate_y(float dist) {
	Frame* frm = &teapot_frame;
	Real vtrans[3] = {0, dist, 0};
	if(local_transform) {
		// transform vector by current rotation matrix
		geom_vector3_matrix3_mul(vtrans, frm->basis, vtrans);
	}
	geom_vector3_add(frm->origin, vtrans, frm->origin);
}

static void translate_z(float dist) {
	Frame* frm = &teapot_frame;
	Real vtrans[3] = {0, 0, dist};
	geom_vector3_add(frm->origin, vtrans, frm->origin);
}

static void rotate_x(float angle) {
	Frame* frm = &teapot_frame;
	Real mat[3*3];
	if(local_transform) {
		// transform x vector by current rotation matrix
		Real vx[3] = {1, 0, 0};
		Real vtx[3];
		geom_vector3_matrix3_mul(vx, frm->basis, vtx);
		// get a matrix with rotation around that transformed vector
		geom_matrix3_new_rot(vtx, angle, mat);
	}
	else {
		geom_matrix3_new_rotx(angle, mat);
	}
	geom_matrix3_mul(frm->basis, mat, frm->basis);
}

static void rotate_y(float angle) {
	Frame* frm = &teapot_frame;
	Real mat[3*3];
	if(local_transform) {
		// transform y vector by current rotation matrix
		Real vy[3] = {0, 1, 0};
		Real vty[3];
		geom_vector3_matrix3_mul(vy, frm->basis, vty);
		// get a matrix with rotation around that transformed vector
		geom_matrix3_new_rot(vty, angle, mat);
	}
	else {
		geom_matrix3_new_roty(angle, mat);
	}
	geom_matrix3_mul(frm->basis, mat, frm->basis);
}

static void rotate_z(float angle) {
	Frame* frm = &teapot_frame;
	Real mat[3*3];
	geom_matrix3_new_rotz(angle, mat);
	geom_matrix3_mul(frm->basis, mat, frm->basis);
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
                translate_y(trans_dist*dir);
            else
                rotate_x(-rot_angle*dir);
            break;
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            dir = key == GLUT_KEY_RIGHT ? 1 : -1;
            if(translate)
                translate_x(trans_dist*dir);
            else
                rotate_y(rot_angle*dir);
            break;
            
        default:
            break;
	}
}

// ===== drawing

static void draw_teapot() {
	glColor3fv(teapot_color);
	glutSolidTeapot(1.0);
}

static void draw_new_tri(Real v1[3], Real v2[3], Real v3[3]){
    
    Real norm[3];
    
//    glPushMatrix();
    glBegin(GL_TRIANGLES);
    geom_calc_normal(v1, v2, v3, norm);
    glNormal3rv(norm);
    glVertex3rv(v1);
    glVertex3rv(v2);
    glVertex3rv(v3);
    glEnd();
//    glPopMatrix();
}

static void display() {
    
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	// set camera viewpoint
	camera_lookat();
        
    Real vert1[3];
	Real vert2[3];
	Real vert3[3];
    
    // 	printf("Size of array: %d \n", sizeof(faces));
    
    
    
	glPushMatrix();
    
    // draw teapot
//    draw_teapot();
    
    Real center[3] = {0.0, 0.0, 5.0};
    
    for(int x = 0; x<numOfFaces; x+=3){
        vert1[0] = vertices[((faces[x]-1)*3)];
        vert1[1] = vertices[((faces[x]-1)*3)+1];
        vert1[2] = vertices[((faces[x]-1)*3)+2] + center[2];
        
        vert2[0] = vertices[((faces[x+1]-1)*3)];
        vert2[1] = vertices[((faces[x+1]-1)*3)+1];
        vert2[2] = vertices[((faces[x+1]-1)*3)+2] + center[2];
        
        vert3[0] = vertices[((faces[x+2]-1)*3)];
        vert3[1] = vertices[((faces[x+2]-1)*3)+1];
        vert3[2] = vertices[((faces[x+2]-1)*3)+2] + center[2];
        
        draw_new_tri(vert1, vert2, vert3);
    }
    
    // draw teapot matrix transform
    glPushMatrix();
    glLoadIdentity();	 	// remove effect of camera transform
    glPopMatrix();
    
	glPopMatrix();
    
	glFlush();
	glutSwapBuffers();
    
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

static void parse_obj(char* fileName){
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

int main(int argc, char* argv[]) {
    
	glutInit(&argc, argv);
    
	glutInitWindowSize(init_win_width, init_win_height);
	win_id = glutCreateWindow(win_title);
	if(win_id <= 0) {
		fprintf(stderr, "Error: glutCreateWindow() returned %d\n", win_id);
		exit(1);
	}
    
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

