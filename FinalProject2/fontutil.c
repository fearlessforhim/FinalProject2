// fontutil.c	:	font utility functions

#include <string.h>

#include "fontutil.h"

// TODO: add support for multiple fonts

#define FONT_SIZE_PTS 11
#define FONT_WIDTH_PIXELS 9
#define FONT_HEIGHT_PIXELS 15

int fontutil_char_width() {
	return FONT_WIDTH_PIXELS;
}

int fontutil_char_height() {
	return FONT_HEIGHT_PIXELS;
}

int fontutil_string_width(const char* s) {
	return strlen(s) * 10;
}

void fontutil_draw_string(const char* s) {
	int i;
	int len = strlen(s);
	for(i = 0; i < len; ++i)
		fontutil_draw_char(s[i]);
}

