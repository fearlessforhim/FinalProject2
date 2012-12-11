// fontutil_glut.c	:	glut-specific font utility functions

// TODO: add support for multiple glut fonts

#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>

#define GLUT_FONT GLUT_BITMAP_9_BY_15

void fontutil_draw_char(char c) {
	glutBitmapCharacter(GLUT_FONT, c);
}


