//
//  texturing.c
//  FinalProject2
//
//  Created by Jonathan Cameron on 12/12/12.
//  Copyright (c) 2012 Jonathan Cameron. All rights reserved.
//

#include "texturing.h"

static const char* play_file = "wm_play.png";
static struct Image play_image;
static GLuint play_texture = 0;

static const char* pause_file = "wm_pause.png";
static struct Image pause_image;
static GLuint pause_texture = 0;

static void init_texture(const char* img_file, struct Image* img, GLuint* tex_id) {
	// load image
	memset(img, 0, sizeof(struct Image));
	int rc = image_load(img_file, img);
	if(rc != IMAGEUTIL_NOERROR) {
		printf("failed loading img: %s\n", img_file);
		return;
	}
    
	// create texture object
	glGenTextures(1, tex_id);
    
	// bind tex object to current state
	glBindTexture(GL_TEXTURE_2D, *tex_id);
    
	// define tex image
	glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 img->width,
                 img->height,
                 0,
                 img->gl_format,
                 GL_UNSIGNED_BYTE,
                 img->pixels);
	
	// set params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void init_textures() {
	init_texture(play_file, &play_image, &play_texture);
	init_texture(pause_file, &pause_image, &pause_texture);
	glBindTexture(GL_TEXTURE_2D, 0);
}