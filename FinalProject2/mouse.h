#ifndef _mouse_h_
#define _mouse_h_

typedef enum {
	MOUSE_INVALID_ID = -1,
	MOUSE_BUTTON_LEFT,	
	MOUSE_BUTTON_MIDDLE,	
	MOUSE_BUTTON_RIGHT,	
	MOUSE_BUTTON_DOWN,	
	MOUSE_BUTTON_UP,	
} MouseId;

int mouse_button(int framework_button_id);
int mouse_state(int framework_state_id);

#endif
