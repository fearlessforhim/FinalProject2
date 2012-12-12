// imageutil.c	:	image utility functions

#include <stdlib.h>
#include <string.h>

#include <OpenGL/gl.h>
#include <GLUT/GLUT.h>

#include "stb_image.h"

#include "imageutil.h"

int image_load(const char* fname, struct Image* img) {
	// load
	img->pixels = stbi_load(fname, &img->width, &img->height, &img->num_components, 0);
	if(img->num_components == 0) {
		return IMAGEUTIL_ERROR_ZERO_COMPONENTS;
	}
	// report
	printf("loaded image from file %s: %dx%d pixels, %d components\n", fname, img->width, img->height, img->num_components);

	// flip vertically
	unsigned char* buf = malloc(img->width * img->height * img->num_components);
	unsigned char* src = img->pixels;
	unsigned char* dst = buf + img->height * img->width * img->num_components;
	int i;
	for(i = 0; i < img->height; ++i) {
		dst -= img->width * img->num_components;
		memcpy(dst, src, img->width * img->num_components);
		src += img->width * img->num_components;
	}
	free(img->pixels);
	img->pixels = buf;

	// determine gl format
	switch(img->num_components) {
	case 1: img->gl_format = GL_LUMINANCE; break;
	case 2: img->gl_format = GL_LUMINANCE_ALPHA; break;
	case 3: img->gl_format = GL_RGB; break;
	case 4: img->gl_format = GL_RGBA; break;
	default:
		return IMAGEUTIL_ERROR_UNKNOWN_GL_FORMAT;
	}
	return IMAGEUTIL_NOERROR;
}

