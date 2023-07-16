#pragma once
#include <Windows.h>
#include <cassert>
#include <cstdio>
#include "../core/maths.h"

typedef struct mouse
{
	// for camera orbit
	vec2 orbit_pos;//������������λ�ã����ܱ�ʾ��άƽ���ϵ����������
	vec2 orbit_delta;//��������������������ʾ�������ı仯��
	// for first-person view (diabled now)
	vec2 fv_pos;//���ڵ�һ�˳��ӽǵ�λ�ã�Ŀǰ�����ã���˿���û��ʵ������
	vec2 fv_delta;//���ڵ�һ�˳��ӽǵ�������ͬ��������
	// for mouse wheel
	float wheel_delta;//���������ֵ���������һ������������ʾ���ֵı仯��
}mouse_t;

typedef struct window 
{
	HWND h_window;//��ʾ���ڵľ��(HWND,��window handle)
	HDC mem_dc;//��ʾ�ڴ��豸��������(HDC����Device Context)
	HBITMAP bm_old;//��ʾ�ɵ�λͼ���(HBITMAP����Bitmap Handle)
	HBITMAP bm_dib;//��ʾ�豸�޹�λͼ���(HBITMAP,��Bitmap Handle)
	unsigned char* window_fb;//��ʾ����ǰ�����ָ�룬ָ��һ���޷����ַ���(unsigned char)����
	int width;//��ʾ���ڵĿ��
	int height;//��ʾ���ڵĸ߶�

	int text_height;
	int text_width;
	HWND text_handle;

	char keys[512];//��ʾ����״̬�����飬����Ϊ512�����ڴ洢������״̬
	char buttons[2];//��ʾ��갴ť״̬�����飬����Ϊ2�����ڴ洢������갴ť��״̬��left button��0�� right button��1
	int is_close;//��ʾ�����Ƿ񱻹رյı�־������ֵΪ����ʱ����ʾ�����ѹر�
	mouse_t mouse_info;//��ʾ�����Ϣ�Ľṹ����������������������Ϣ����������λ�á������Լ�����������
}window_t;

extern window_t* window;

int window_init(int width, int height, const char* title);
int window_destroy();
void window_draw(unsigned char* framebuffer);
void window_draw_text(window_t* window, char* text);
void msg_dispatch();
vec2 get_mouse_pos();
float platform_get_time(void);
