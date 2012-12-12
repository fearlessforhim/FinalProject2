//
//  texturing.h
//  FinalProject2
//
//  Created by Jonathan Cameron on 12/12/12.
//  Copyright (c) 2012 Jonathan Cameron. All rights reserved.
//

#ifndef FinalProject2_texturing_h
#define FinalProject2_texturing_h



#endif

#include <stdio.h>
#include <string.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>

#include "imageutil.h"
#include "stb_image.h"

static void init_texture(const char* img_file, struct Image* img, GLuint* tex_id);

void init_textures();