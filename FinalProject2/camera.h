#ifndef _camera_h_
#define _camera_h_

#include "precision.h"

// sets camera parameters
void camera_set(const Real* eye, const Real* cen, const Real* up);

// returns camera parameters
void camera_get(Real* eye_ret, Real* cen_ret, Real* up_ret);

// call from display func, does gluLookAt() for camera pos
void camera_lookat();

// call from keyboard func, returns true if camera key used
int camera_keyboard(int key);

// call from glut mouse func, returns true if mouse input used
int camera_mouse(int button, int state, int x, int y);

// call from glut motion func, returns true if mouse input used
int camera_motion(int x, int y);

// util function, sets perspective projection (optional, not needed for camera functionality)
void camera_set_projection();

// joystick input
int camera_joystick(int axis, int value);

// needed to update joystick input
void camera_tick();

#endif
