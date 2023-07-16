#pragma once
#include <Windows.h>
#include <cassert>
#include <cstdio>
#include "../core/maths.h"

typedef struct mouse
{
	// for camera orbit
	vec2 orbit_pos;//用于相机轨道的位置，可能表示二维平面上的坐标或向量
	vec2 orbit_delta;//用于相机轨道的增量，表示相机轨道的变化量
	// for first-person view (diabled now)
	vec2 fv_pos;//用于第一人称视角的位置，目前被禁用，因此可能没有实际意义
	vec2 fv_delta;//用于第一人称视角的增量，同样被禁用
	// for mouse wheel
	float wheel_delta;//用于鼠标滚轮的增量，是一个浮点数，表示滚轮的变化量
}mouse_t;

typedef struct window 
{
	HWND h_window;//表示窗口的句柄(HWND,即window handle)
	HDC mem_dc;//表示内存设备的上下文(HDC，即Device Context)
	HBITMAP bm_old;//表示旧的位图句柄(HBITMAP，即Bitmap Handle)
	HBITMAP bm_dib;//表示设备无关位图句柄(HBITMAP,即Bitmap Handle)
	unsigned char* window_fb;//表示窗口前缓冲的指针，指向一个无符号字符型(unsigned char)数组
	int width;//表示窗口的宽度
	int height;//表示窗口的高度

	int text_height;
	int text_width;
	HWND text_handle;

	char keys[512];//表示键盘状态的数组，长度为512，用于存储按键的状态
	char buttons[2];//表示鼠标按钮状态的数组，长度为2，用于存储左右鼠标按钮的状态，left button—0， right button—1
	int is_close;//表示窗口是否被关闭的标志，当该值为非零时，表示窗口已关闭
	mouse_t mouse_info;//表示鼠标信息的结构体变量，包含了鼠标的相关信息，如相机轨道位置、增量以及滚轮增量等
}window_t;

extern window_t* window;

int window_init(int width, int height, const char* title);
int window_destroy();
void window_draw(unsigned char* framebuffer);
void window_draw_text(window_t* window, char* text);
void msg_dispatch();
vec2 get_mouse_pos();
float platform_get_time(void);
