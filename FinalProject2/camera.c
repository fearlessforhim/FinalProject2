// camera.c	:	camera libray
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <OpenGL/glu.h>
#include <OpenGL/OpenGL.h>

//#include "keyboard/keyboard.h"
//#include "mouse/mouse.h"

#include "precision.h"
//#include "trace/trace.h"
#include "geom.h"

#include "camera.h"

/*
 #define KEY_TRANS_x SDLK_a
 #define KEY_TRANS_X SDLK_d
 #define KEY_TRANS_y SDLK_MINUS
 #define KEY_TRANS_Y SDLK_PLUS
 #define KEY_TRANS_z SDLK_w
 #define KEY_TRANS_Z SDLK_s
 #define KEY_ROT_x SDLK_DOWN
 #define KEY_ROT_X SDLK_UP
 #define KEY_ROT_y SDLK_LEFT
 #define KEY_ROT_Y SDLK_RIGHT
 */
#define KEY_TRANS_x 'a'
#define KEY_TRANS_X 'd'
#define KEY_TRANS_y '-'
#define KEY_TRANS_Y '+'
#define KEY_TRANS_z 'w'
#define KEY_TRANS_Z 's'
#define KEY_ROT_x 'k'
#define KEY_ROT_X 'i'
#define KEY_ROT_y 'j'
#define KEY_ROT_Y 'l'
//
//#define KEY_PRINT_CAM KBD_KEY_p
//#define KEY_HELP GLUT_KEY_F1
#define JOY_MAX 32767
#define JOY_THRESHOLD 1000

static char camera_help[] = \
"--- camera keys:\n"\
"  translate -x:  'a'\n"\
"  translate +x:  'd'\n"\
"  translate -y:  '-'\n"\
"  translate +y:  '+'\n"\
"  translate -z:  'w'\n"\
"  translate +z:  's'\n"\
"  rotate    -x:  up\n"\
"  rotate    +x:  down\n"\
"  rotate    -y:  left\n"\
"  rotate    +y:  right\n"\
"--- camera mouse:\n"\
"  translate -x:  rb_down + move left\n"\
"  translate +x:  rb_down + move right\n"\
"  translate -z:  rb_down + move up\n"\
"  translate +z:  rb_down + move down\n"\
"  rotate    -x:  lb_down + move up\n"\
"  rotate    +x:  lb_down + move down\n"\
"  rotate    -y:  lb_down + move left\n"\
"  rotate    +y:  lb_down + move right\n"\
"\n"\
;

static Real eye[] = {2.0, 1.0, 0.0};
static Real cen[] = {0, 0, 5.0};
static Real up[] = {0, 1, 0};

// keyboard
static Real key_trans_rate = 0.5;
static Real key_rot_rate = 5.0;

// mouse
//static int mouse_left_button_state = MOUSE_BUTTON_UP;
//static int mouse_right_button = MOUSE_BUTTON_UP;
static int prev_motion_x = -1;
static int prev_motion_y = -1;
static Real mouse_trans_rate = 0.05;
static Real mouse_rot_rate = 0.5;
static int shift_key_down = 0;
static int ctrl_key_down = 0;

// joystick
static Real joy_trans[3] = {0, 0, 0};
static Real joy_rotx = 0;
static Real joy_roty = 0;
static Real joy_trans_rate = 1.0;
static Real joy_roty_rate = 3.0;
static Real joy_rotx_rate = 1.0;

// debug
static int _camera_debug = 0;

void camera_debug(int enable) {
	_camera_debug = enable;
}

/*  returns matrix representing param cam rotation where cam up dir is projected onto alignment with world y axis,
 or nil if cam up dir is perpedicular to world y axis
 */
static int get_projected_frame(Real* look_dir, Real* mat_ret) {
	// convert translation to adjusted camera frame, where adjustment is projection of look_dir onto xz plane,
	// up_dir is aligned with world y axis
	int rc = 0;
	Real proj_look_dir[3];
	rc |= geom_vector3_copy(look_dir, proj_look_dir);
	proj_look_dir[1] = 0;
	rc |= geom_vector3_normalize(proj_look_dir);
	if( geom_vector3_magnitude(proj_look_dir) == 0 )
		return GEOM_ERR_ZERO_VECTOR;
	Real vy[3] = {0, 1, 0};
	Real vx[3];
	rc |= geom_vector3_cross(proj_look_dir, vy, vx);
	Real vz[3];
	rc |= geom_vector3_scalar_mul(proj_look_dir, -1, vz);
	Real Mr[9] = {vx[0], vx[1], vx[2],
		vy[0], vy[1], vy[2],
		vz[0], vz[1], vz[2]};
	rc |=geom_matrix3_copy( Mr, mat_ret);
	return rc;
}

static int get_look_dir(Real* look_dir_ret) {
	// get look dir
	geom_vector3_sub(cen, eye, look_dir_ret);
	int rc = geom_vector3_normalize(look_dir_ret);
	return rc;
}

static void camera_translate(const Real* v) {
    
	// debug
	if(_camera_debug) {
		printf("camera_translate(): v: (%f, %f, %f)\n", (float)v[0], (float)v[1], (float)v[2]);
		fflush(stdout);
	}
    
	// tranform trans vector by current look and up dirs
	Real look_dir[3];
	get_look_dir(look_dir);
    
	Real mp[9];
	get_projected_frame(look_dir, mp);
	Real vt[3];
	geom_vector3_matrix3_mul(v, mp, vt);
	geom_vector3_add(eye, vt, eye);
	geom_vector3_add(cen, vt, cen);
}

static void camera_rotate_x(Real angle_deg) {
    
	// get look dir and mag
	Real look_dir[3];
	geom_vector3_sub(cen, eye, look_dir);
	Real look_mag = geom_vector3_magnitude(look_dir);
	geom_vector3_normalize(look_dir);
    
	// rotate both look dir and up dir about the x axis
	Real vrot[3];
	geom_vector3_cross(look_dir, up, vrot);
	Real mrot[9];
	geom_matrix3_new_rot(vrot, angle_deg, mrot);
	geom_vector3_matrix3_mul(look_dir, mrot, look_dir);
	geom_vector3_matrix3_mul(up, mrot, up);
    
	// project new cen from rotated look_dir
	Real new_look[3];
	geom_vector3_set(new_look, look_mag*look_dir[0], look_mag*look_dir[1], look_mag*look_dir[2]);
	geom_vector3_add(eye, new_look, cen);
}

static void camera_rotate_y(Real angle_deg) {
	// get look dir and mag
	Real look_dir[3];
	geom_vector3_sub(cen, eye, look_dir);
	Real look_mag = geom_vector3_magnitude(look_dir);
	geom_vector3_normalize(look_dir);
    
	// rotate both look dir and up dir about the y axis
	Real vy[] = {0, 1, 0};
	Real mrot[9];
	geom_matrix3_new_rot(vy, angle_deg, mrot);
	geom_vector3_matrix3_mul(look_dir, mrot, look_dir);
	geom_vector3_matrix3_mul(up, mrot, up);
    
	// project new cen from rotated look_dir
	Real new_look[3];
	geom_vector3_set(new_look, look_mag*look_dir[0], look_mag*look_dir[1], look_mag*look_dir[2]);
	geom_vector3_add(eye, new_look, cen);
}

void camera_set(const Real* new_eye, const Real* new_cen, const Real* new_up) {
	geom_vector3_copy(new_eye, eye);
	geom_vector3_copy(new_cen, cen);
	geom_vector3_copy(new_up, up);
}

void camera_get(Real* eye_ret, Real* cen_ret, Real* up_ret) {
	geom_vector3_copy(eye, eye_ret);
	geom_vector3_copy(cen, cen_ret);
	geom_vector3_copy(up, up_ret);
}

static void print_camera_info() {
	camera_get(eye, cen, up);
	printf("static Real init_eye[] = {%f, %f, %f};\nstatic Real init_cen[] = {%f, %f, %f};\nstatic Real init_up[] = {%f, %f, %f};\n",
           eye[0],eye[1],eye[2],
           cen[0],cen[1],cen[2],
           up[0],up[1],up[2]);
}

static void print_camera_help() {
	printf("%s", camera_help);
}

int camera_keyboard(int key) {
	// debug
	if(_camera_debug) {
		printf("camera_keyboard(): key: %d\n", key);
		fflush(stdout);
	}
    
//	int shift_key_down = keyboard_get_modifiers() & KBD_MOD_SHIFT;
//	int ctrl_key_down = keyboard_get_modifiers() & KBD_MOD_CTRL;
	
	switch(key) {
        case KEY_TRANS_x: { Real v[3] = {-key_trans_rate, 0, 0}; camera_translate(v); } break;
        case KEY_TRANS_X: { Real v[3] = {key_trans_rate, 0, 0}; camera_translate(v); } break;
        case KEY_TRANS_y: { Real v[3] = {0, -key_trans_rate, 0}; camera_translate(v); } break;
        case KEY_TRANS_Y: { Real v[3] = {0, key_trans_rate, 0}; camera_translate(v); } break;
        case KEY_TRANS_z: { Real v[3] = {0, 0, -key_trans_rate}; camera_translate(v); } break;
        case KEY_TRANS_Z: { Real v[3] = {0, 0, key_trans_rate}; camera_translate(v); } break;
        case KEY_ROT_x: { camera_rotate_x(-key_rot_rate); } break;
        case KEY_ROT_X: { camera_rotate_x( key_rot_rate); } break;
        case KEY_ROT_y: { camera_rotate_y( key_rot_rate); } break;
        case KEY_ROT_Y: { camera_rotate_y(-key_rot_rate); } break;
//        case KEY_PRINT_CAM:
//            print_camera_info();
//            break;
//        case KEY_HELP: print_camera_help(); break;
            /*
             case KEY_TRANS_z: return camera_keyboard(KEY_TRANS_z, x, y);
             case KEY_TRANS_Z: return camera_keyboard(KEY_TRANS_Z, x, y);
             case KEY_ROT_y:
             if(ctrl_key_down)
             return camera_keyboard(TRANS_x, x, y);
             else
             return camera_keyboard(ROT_y, x, y);
             case KEY_ROT_Y:
             if(ctrl_key_down)
             return camera_keyboard(TRANS_X, x, y);
             else
             return camera_keyboard(ROT_Y, x, y);
             */
        default:
            return 0;
	}
	return 1;
}

int camera_mouse(int button, int state, int x, int y) {
	// debug
//	if(_camera_debug) {
//		printf("camera_mouse(): button: %d, state: %d, x: %d, y: %d\n", button, state, x, y);
//		fflush(stdout);
//	}
//    
//	switch(button) {
//        case MOUSE_BUTTON_LEFT:
//            mouse_left_button_state = state;
//			prev_motion_x = -1;
//			prev_motion_y = -1;
//            break;
//        case MOUSE_BUTTON_RIGHT:
//            mouse_right_button = state;
//			prev_motion_x = -1;
//			prev_motion_y = -1;
//            break;
//        default: break;
//            //		printf("camera_mouse(): button: %d, state: %d\n", button, state);
//	}
	return 0;
}

int camera_motion(int x, int y) {
	// debug
	if(_camera_debug) {
		printf("camera_motion(): x: %d, y: %d\n", x, y);
		fflush(stdout);
	}
    
    //	ctrl_key_down = glutGetModifiers() & GLUT_ACTIVE_CTRL;
	int rc = 0;
//	if(mouse_left_button_state == MOUSE_BUTTON_DOWN) {
//		if(prev_motion_x > 0) {
//			int dx = x - prev_motion_x;
//			int dy = y - prev_motion_y;
//            camera_rotate_y(dx * mouse_rot_rate);
//            camera_rotate_x(dy * mouse_rot_rate);
//		}
//		rc = 1;
//	}
//	else if(mouse_right_button == MOUSE_BUTTON_DOWN) {
//		if(prev_motion_x > 0) {
//			int dx = x - prev_motion_x;
//			int dy = y - prev_motion_y;
//			// can't portably detect this with GLUT :(
//			if(ctrl_key_down) {
//				Real v[3] = {-mouse_trans_rate * dx, mouse_trans_rate * -dy, 0};
//				camera_translate(v);
//			}
//			else {
//				Real v[3] = {-mouse_trans_rate * dx, 0, mouse_trans_rate * -dy};
//				camera_translate(v);
//			}
//		}
//		rc = 1;
//	}
	prev_motion_x = x;
	prev_motion_y = y;
	return rc;
}

int camera_joystick(int axis, int value) {
	int rc = 0;
	float norm_val = 0;
	if(fabs(value) > JOY_THRESHOLD)
		norm_val = (float)value / (float)JOY_MAX;
	switch(axis) {
            // move x
        case 0:
		{
            //			Real v[3] = {trans_rate*norm_val, 0, 0};
            //			camera_translate(v);
			float x = joy_trans_rate*norm_val;
			joy_trans[0] = x;
            //printf("camera_joystick(): move x %f: new joy_trans: [%f, %f, %f]\n", x, joy_trans[0], joy_trans[1], joy_trans[2]);
		}
            rc = 1;
            break;
            // move z
        case 1:
		{
			float z = joy_trans_rate*norm_val;
			joy_trans[2] = z;
            //printf("camera_joystick(): move z %f: new joy_trans: [%f, %f, %f]\n", z, joy_trans[0], joy_trans[1], joy_trans[2]);
			rc = 1;
		}
            break;
            // look y
        case 2:
		{
			float y = -joy_roty_rate*norm_val;
			joy_roty = y;
		}
            break;
            // look x
        case 3:
		{
			float x = -joy_rotx_rate*norm_val;
			joy_rotx = x;
		}
            break;
        default:
            break;
	}
	return rc;
}

void camera_tick() {
	camera_translate(joy_trans);
	camera_rotate_y(joy_roty);
	camera_rotate_x(joy_rotx);
}

void camera_lookat() {
    printf("Eye at: %f %f %f\n", eye[0], eye[1], eye[2]);
	gluLookAt(
              eye[0], eye[1], eye[2],
              cen[0], cen[1], cen[2],
              up[0], up[1], up[2]);
}

