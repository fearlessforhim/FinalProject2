#ifndef _grutil_h_
#define _grutil_h_

#include <stddef.h>

#include "precision.h"

// draw simple wireframe axes
void grutil_draw_axes(Real axis_length);

// stats functions
void grutil_inc_verts_drawn(size_t n);
void grutil_clear_verts_drawn();
size_t grutil_get_verts_drawn();

// gl errors
void grutil_check_gl_errors();

#endif
