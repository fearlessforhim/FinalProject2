// mouse_glut.c
#include <stdio.h>

#include <GLUT/glut.h>


#include "mouse.h"

int mouse_button(int framework_button_id) {
	switch(framework_button_id) {
	case GLUT_LEFT_BUTTON: return MOUSE_BUTTON_LEFT;
	case GLUT_MIDDLE_BUTTON: return MOUSE_BUTTON_MIDDLE;
	case GLUT_RIGHT_BUTTON: return MOUSE_BUTTON_RIGHT;
	default: return MOUSE_INVALID_ID;;
	}
}

int mouse_state(int framework_state_id) {
	switch(framework_state_id) {
	case GLUT_UP: return MOUSE_BUTTON_UP;
	case GLUT_DOWN: return MOUSE_BUTTON_DOWN;
	default: return MOUSE_INVALID_ID;;
	}
}
