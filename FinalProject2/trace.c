#include <stdio.h>
#include <stdarg.h>

#include "trace.h"

static const char* _file = NULL;
static int _line = 0;
static int _trace_enabled = 1;

void trace_on(){_trace_enabled = 1;}
void trace_off(){_trace_enabled = 0;}
int trace_loc(const char* file, int line) {
	_file = file;
	_line = line;
	return 1;
}

int _trace(const char* fmt, ...) {
	if(!_trace_enabled)
		return 0;
	printf("%s:%d: ", _file, _line);
	va_list va;
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
	fflush(stdout);
	return 1;
}


