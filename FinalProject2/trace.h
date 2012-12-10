#ifndef _trace_h_
#define _trace_h_

// this gives compiler warning 'value computed is not used', but don't see a better alternative that lets me do var macro args for _trace()
#define trace trace_loc(__FILE__, __LINE__) && _trace

// this fails on non-braced if statements
//#define trace trace_loc(__FILE__, __LINE__); _trace

void trace_on();
void trace_off();
int trace_loc(const char* file, int line);
int _trace(const char* fmt, ...);

#endif
