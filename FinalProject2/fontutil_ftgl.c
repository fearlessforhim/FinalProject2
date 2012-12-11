// fontutil_ftgl.c	:	font utility functions - ftgl version

// TODO: add support for multiple ttf fonts

#include <string.h>

#include "FTGL/ftgl.h"

#include "fontutil.h"

#define FONT_SIZE_PTS 11
#define FONT_WIDTH_PIXELS 9
#define FONT_HEIGHT_PIXELS 15

static FTGLfont *font = NULL;
static char* font_file = "FreeMono.ttf";

static void init_font() {
	font = ftglCreatePixmapFont(font_file);
	if(!font) {
		fprintf(stderr, "ftglCreatePixmapFont(%s) failed\n", font_file);
		exit(1);
	}
	ftglSetFontFaceSize(font, FONT_SIZE_PTS, 0);
}

void fontutil_draw_char(char c) {
	if(font == NULL)
		init_font();
	char s[2];
	s[0] = c;
	s[1] = '\0';
	ftglRenderFont(font, s, FTGL_RENDER_ALL);
}

