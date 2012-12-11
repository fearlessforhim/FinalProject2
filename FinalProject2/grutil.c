// grutil.c	: grutil library module
#include <stdio.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

/*
#ifdef PLATFORM_OSX
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif
*/

#include "geom.h"
#include "fontutil.h"

#include "grutil.h"

static size_t num_verts_drawn = 0;

void grutil_draw_axes(Real axis_length) {
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
//	glLineWidth(line_width);
	Real shade = 1.0;
	int i;
	for(i = 0; i < 3; ++i) {
		Real axis[3] = {i==0?1:0, i==1?1:0, i==2?1:0};
		Real color[3];
		geom_vector3_scalar_mul(axis, shade, color);
		glColor3fv(color);
		glBegin(GL_LINES);
			Real v[3];
			geom_vector3_scalar_mul(axis, axis_length, v);
			glVertex3fv(v);
			geom_vector3_scalar_mul(axis, -axis_length, v);
			glVertex3fv(v);
		glEnd();
		// draw label
		geom_vector3_scalar_mul(axis, axis_length*1.05, v);
		glRasterPos3f(v[0], v[1], v[2]);
		char label[3]="\0\0"; 
		label[0] = i==0?'x':i==1?'y':'z';
		fontutil_draw_string(label);
	}
	glPopAttrib();
}

void grutil_inc_verts_drawn(size_t n){num_verts_drawn += n;}
void grutil_clear_verts_drawn(){num_verts_drawn = 0;}
size_t grutil_get_verts_drawn(){return num_verts_drawn;} 

void grutil_check_gl_errors() {
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR) {
    printf("GL Error %d: %s\n", err, gluErrorString(err));
  }
}

