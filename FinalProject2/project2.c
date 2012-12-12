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
#include <math.h>
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>

#include "precision.h"
#include "geom.h"
#include "camera.h"
#include "grid_terrain.h"
#include "imageutil.h"
#include "texturing.h"

//#include "mvm.h"

#define FPS 60
#define KEY_ESC 27
#define KEY_RESET_TRANSFORM 'r'
#define KEY_ACTION ' '

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
int currentTime = 0;
int previousTime = 0;
int timeInterval = 0;
int elapsedInSecond = 0;
int fps = 0;

//object arrays
static Real **vertices;
static int **faces;
static int *numOfVerts;
static int *numOfFaces;
static int objectVertNum;
static int objectFaceNum;
static int numOfObjects = 1;
static int numOfBars = 10;
//position
static Real sphereCenter[3] = {0.0, 1.0, 3.0};

//drawing
Real norm[3];
Real vert1[3];
Real vert2[3];
Real vert3[3];
Real center[3];
int texture_num = 2;
bool draw_top = false;



//movement
Real barHeight = 0;
Real ballSpeed = .5;
Real xDist, yDist, zDist, dist;
Real actionCenter[3] = {-1, 1, 24};
static bool action = true;
static Real eyePos[3];
static Real cenPos[3];
static Real upPos[3];

// terrain
static Real ground_color[3] = {0.25, 0.566, 0.25};
static Real grid_unit = 10;
static int num_adj_levels = 1;

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
            
        case KEY_ACTION:
            camera_get(eyePos, cenPos, upPos);
            xDist = fabs(eyePos[0] - actionCenter[0]);
            yDist = fabs(eyePos[1] -actionCenter[1]);
            zDist = fabs(eyePos[2] - actionCenter[2]);
            dist = sqrtf(powf(xDist, 2.0) + powf(yDist, 2.0) + powf(zDist, 2.0));
            
            if(dist < 2){
                if(action){
                    texture_num = 1;
                    action = false;
                }else{
                    texture_num = 2;
                    action = true;
                }
            }
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

// ===== math

static Real get_bar_height(Real c[3]){
    xDist = fabs(sphereCenter[0] - c[0]);
    zDist = fabs(sphereCenter[2] - c[2]);
    dist = sqrtf(powf(xDist, 2.0)+powf(zDist, 2.0));
    return dist;
}

static void move_ball(){
    if(sphereCenter[0] > 16 || sphereCenter[0] < -16){
        ballSpeed = -ballSpeed;
    }
    
    sphereCenter[0] += ballSpeed;
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

static void draw_texture_tri(Real v1[3], Real v2[3], Real v3[3], int textureNum){
    glBindTexture(GL_TEXTURE_2D, textureNum);
    if(!draw_top){
        glBegin(GL_TRIANGLES);
        geom_calc_normal(v1, v2, v3, norm);
        glNormal3rv(norm);
        glTexCoord2f(0,0);
        glVertex3rv(v1);
        glTexCoord2f(1, 0);
        glVertex3rv(v2);
        glTexCoord2f(0, 1);
        glVertex3rv(v3);
        glEnd();
        draw_top = true;
    }else{
        glBegin(GL_TRIANGLES);
        geom_calc_normal(v1, v2, v3, norm);
        glNormal3rv(norm);
        glTexCoord2f(1,0);
        glVertex3rv(v1);
        glTexCoord2f(1, 1);
        glVertex3rv(v2);
        glTexCoord2f(0, 1);
        glVertex3rv(v3);
        glEnd();
        draw_top = false;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void draw_verts(Real c[3], int objNum){
    
    
    //    printf("SIZE OF vertices: %lu", sizeof(vertices[objNum]));
    
    for(int x = 0; x<numOfFaces[objNum]; x+=3){
        vert1[0] = vertices[objNum][((faces[objNum][x]-1)*3)]+c[0];
        vert1[1] = vertices[objNum][((faces[objNum][x]-1)*3)+1]+c[1];
        vert1[2] = vertices[objNum][((faces[objNum][x]-1)*3)+2] + c[2];
        
        vert2[0] = vertices[objNum][((faces[objNum][x+1]-1)*3)]+c[0];
        vert2[1] = vertices[objNum][((faces[objNum][x+1]-1)*3)+1]+c[1];
        vert2[2] = vertices[objNum][((faces[objNum][x+1]-1)*3)+2] + c[2];
        
        vert3[0] = vertices[objNum][((faces[objNum][x+2]-1)*3)]+c[0];
        vert3[1] = vertices[objNum][((faces[objNum][x+2]-1)*3)+1]+c[1];
        vert3[2] = vertices[objNum][((faces[objNum][x+2]-1)*3)+2] + c[2];
        
        draw_new_tri(vert1, vert2, vert3);
    }
}

static void draw_cube(Real c[3], Real h, Real w, bool withTexture){
    draw_top = false;
        w = w/2;
        Real mesh[] = {
            //draw front side
            c[0]-w, c[1], c[2]+w,
            c[0]+w, c[1], c[2]+w,
            c[0]-w, c[1]+h, c[2]+w,
            c[0]+w, c[1], c[2]+w,
            c[0]+w, c[1]+h, c[2]+w,
            c[0]-w, c[1]+h, c[2]+w,
            //draw back side
            c[0]-w, c[1], c[2]-w,
            c[0]+w, c[1], c[2]-w,
            c[0]-w, c[1]+h, c[2]-w,
            c[0]+w, c[1], c[2]-w,
            c[0]+w, c[1]+h, c[2]-w,
            c[0]-w, c[1]+h, c[2]-w,
            //draw left side
            c[0]-w, c[1], c[2]+w,
            c[0]-w, c[1], c[2]-w,
            c[0]-w, c[1]+h, c[2]+w,
            c[0]-w, c[1], c[2]-w,
            c[0]-w, c[1]+h, c[2]-w,
            c[0]-w, c[1]+h, c[2]+w,
            //draw right side
            c[0]+w, c[1], c[2]+w,
            c[0]+w, c[1], c[2]-w,
            c[0]+w, c[1]+h, c[2]+w,
            c[0]+w, c[1], c[2]-w,
            c[0]+w, c[1]+h, c[2]-w,
            c[0]+w, c[1]+h, c[2]+w,
            //draw top
            c[0]-w, c[1]+h, c[2]-w,
            c[0]+w, c[1]+h, c[2]-w,
            c[0]-w, c[1]+h, c[2]+w,
            c[0]+w, c[1]+h, c[2]-w,
            c[0]+w, c[1]+h, c[2]+w,
            c[0]-w, c[1]+h, c[2]+w,
            //draw bottom
            c[0]-w, c[1], c[2]-w,
            c[0]+w, c[1], c[2]-w,
            c[0]-w, c[1], c[2]+w,
            c[0]+w, c[1], c[2]-w,
            c[0]+w, c[1], c[2]+w,
            c[0]-w, c[1], c[2]+w,
        };
        
        
        for(int i = 0; i < 12; i++){
            vert1[0] = mesh[(i*9)];
            vert1[1] = mesh[(i*9)+1];
            vert1[2] = mesh[(i*9)+2];
            
            vert2[0] = mesh[(i*9)+3];
            vert2[1] = mesh[(i*9)+4];
            vert2[2] = mesh[(i*9)+5];
            
            vert3[0] = mesh[(i*9)+6];
            vert3[1] = mesh[(i*9)+7];
            vert3[2] = mesh[(i*9)+8];
            
            if(!withTexture){
                draw_new_tri(vert1, vert2, vert3);
            }else{
                draw_texture_tri(vert1, vert2, vert3, texture_num);
            }
        }

    
}

static void display() {
    
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	// set camera viewpoint
	camera_lookat();
    
    center[0] = sphereCenter[0];
    center[1] = sphereCenter[1];
    center[2] = sphereCenter[2];
    
    glColor3f(1.0, 1.0, 1.0);
    
    draw_verts(center, 0);//draw sphere
    
    center[0] = -18.5;
    center[1] = 0;
    center[2] = 0;
    
    for(int i = 0; i < numOfBars; i++){//draw bars
        barHeight = get_bar_height(center);
        draw_cube(center, 30/barHeight, 3, false);
        center[0] += 4;
    }
    
    center[0] = actionCenter[0];
    center[1] = actionCenter[1];
    center[2] = actionCenter[2];
    
    draw_cube(center, .7, .7, true);//draw action block
    
    grid_terrain_draw(num_adj_levels);
    
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
    
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    timeInterval = currentTime - previousTime;
    
    if(timeInterval > 16){
        glutPostRedisplay();
        previousTime = currentTime;
        elapsedInSecond += timeInterval;
        
        if(action)
            move_ball();
    }
    
    if(elapsedInSecond > 1000){
        fps = frame_count;
        
        frame_count = 0;
        elapsedInSecond = 0;
        
        char title[256];
        sprintf(title, "%s - %d fps", win_title, (int)fps);
		glutSetWindowTitle(title);
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
    
    free(current_token);
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
    
    free(current_token);
}




static void init(){
    
    vertices = (Real**)malloc(numOfObjects*sizeof(Real*));//create multidimensinal arrays
    faces = (int**)malloc(numOfObjects*sizeof(int*));
    numOfVerts = (int*)malloc(numOfObjects*sizeof(int));
    numOfFaces = (int*)malloc(numOfObjects*sizeof(int));
    
    getNumOfVertsFaces("sphere.obj", 0);
    
    parse_obj("sphere.obj", 0);
    
    //init terrain
    grid_terrain_set_draw_op(GL_TRIANGLES);
	grid_terrain_set_unit(grid_unit);
	grid_terrain_set_color(ground_color);
	grid_terrain_create();
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
    
    init_textures();
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    numOfVerts = NULL;
    
    glBindTexture(GL_TEXTURE_2D, 0);
	
	glutMainLoop();
    
	return 0;
}

