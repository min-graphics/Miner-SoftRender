#include "./win32.h"

window_t* window = NULL;

/*
hWnd:���ڵľ������ʶ�˸���Ϣ�����Ĵ��ڡ�
msg:��ʾ���ܵ��Ĵ�����Ϣ�ı�ʶ��(Message ID)
wParam:��������Ϣ��صĸ�����Ϣ
lParam:��������Ϣ��صĸ�����Ϣ
*/
//�ں�������ֵ�ͺ�����֮����Լ��� �궨������η����Ա�֤��ȷ�ĺ�������Լ���Ͳ������ݷ�ʽ������CALLBACK���Ա�����Ϊ__stdcall����__cdecl����ȡ���ڱ�����������
/*
__stdcall��һ�ֱ�׼�ĵ���Լ�����ڵ��ú���ʱ�����ɵ��÷����ж�ջ����������
__cdecl��Ĭ�ϵĵ���Լ���������ɱ����ú����Լ����ж�ջ����������
*/
static LRESULT CALLBACK msg_callback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) //�ص���������
{
	switch (msg) 
	{
		case WM_CLOSE: 
			window->is_close = 1; 
			break;
		case WM_KEYDOWN: 
			window->keys[wParam & 511] = 1; //���&511��λ�����������511=111111111��������ȡwParam�ĵ�9λ����ֵ������0��511֮��
			break;
		case WM_KEYUP: 
			window->keys[wParam & 511] = 0; 
			break;
		case WM_LBUTTONDOWN:
			window->mouse_info.orbit_pos = get_mouse_pos();
			window->buttons[0] = 1; 
			break;
		case WM_LBUTTONUP:
			window->buttons[0] = 0; 
			break;
		case WM_RBUTTONDOWN:
			window->mouse_info.fv_pos = get_mouse_pos();
			window->buttons[1] = 1; 
			break;
		case WM_RBUTTONUP:
			window->buttons[1] = 0; 
			break;
		case WM_MOUSEWHEEL:
			window->mouse_info.wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			break;

		default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

///*
//��δ���ʹ����Ԥ����ָ��#ifdef�����ݱ���ʱ�Ƿ����˺�UNICODE���������鳣����
//UNICODE�궨�����߱�����ʹ��Unicode����������Դ���룬��û�ж���UNICODE�궨����Ĭ��ʹ��ASCII���롣
//
//���������UNICODE�궨�壬����WINDOW_CLASS_NAME��WINDOW_ENTRY_NAME������Ϊwchar_t���͵��ַ����������ַ����͵��ַ�������L��ͷ��
//���򱻶���Ϊchar���͵��ַ�������խ�ַ����͵��ַ���������WINDOW_CLASS_NAME�Ǵ�������������WINDOW_ENTRY_NAME�Ǵ��ڵı�������
//
//��������Ŀ����Ϊ���ڲ�ͬ���뻷�����ܹ���ȷ�ض��崰�������ʹ��ڱ���������ȷ���ڲ�ͬ��ϵͳ���ܹ���ȷ�ش������ڡ�
//*/
//#ifdef UNICODE
//static const wchar_t* const WINDOW_CLASS_NAME = L"Class";
//static const wchar_t* const WINDOW_ENTRY_NAME = L"Entry";
//#else
//static const char* const WINDOW_CLASS_NAME = "Class";
//static const char* const WINDOW_ENTRY_NAME = "Entry";
//#endif

/*
	UINT        style;
	WNDPROC     lpfnWndProc;
	int         cbClsExtra;
	int         cbWndExtra;
	HINSTANCE   hInstance;
	HICON       hIcon;
	HCURSOR     hCursor;
	HBRUSH      hbrBackground;
	LPCSTR      lpszMenuName;
	LPCSTR      lpszClassName;
*/
static void register_window_class()
{
	ATOM atom;
	//��ʼ���ṹ��
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;						//���ڷ��
	wc.lpfnWndProc = (WNDPROC)msg_callback;					//�ص�����
	wc.cbClsExtra = 0;										//�����ڴ�����β����һ�����ռ䣬��������Ϊ0
	wc.cbWndExtra = 0;										//�����ڴ���ʵ��β����һ�����ռ䣬��������Ϊ0
	wc.hInstance = GetModuleHandle(NULL);					//��ǰʵ�����
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);				//������ͼ��
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);				//�����ʽ
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//������ʽ
	wc.lpszMenuName = NULL;									//�˵�
	wc.lpszClassName = "window";					//�ô����������

	atom = RegisterClass(&wc); //ע�ᴰ����
	assert(atom != 0);
	((void)(atom));
}

/*
		DWORD      biSize;
		LONG       biWidth;
		LONG       biHeight;
		WORD       biPlanes;
		WORD       biBitCount;
		DWORD      biCompression;
		DWORD      biSizeImage;
		LONG       biXPelsPerMeter;
		LONG       biYPelsPerMeter;
		DWORD      biClrUsed;
		DWORD      biClrImportant;
*/
static void init_bm_header(BITMAPINFOHEADER &bi,int width,int height)
{
	memset(&bi, 0, sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;   //���ϵ���
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = width * height * 4;
}

int window_init(int width, int height, const char *title) 
{
	window = (window_t*)malloc(sizeof(window_t));
	if (window != nullptr)
	{
		memset(window, 0, sizeof(window_t));
		window->is_close = 0;
	}
//��
	window->text_handle = CreateWindow(TEXT("static"), TEXT("static"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, text_width, text_height, window->h_window, HMENU(21), GetModuleHandle(NULL), NULL);
	SetTextColor(GetDC(window->text_handle), RGB(255, 0, 0));
//
	RECT rect = { 0, 0, width, height }; //һ�����η�Χ ��������
	int wx, wy, sx, sy;
	LPVOID ptr; //����void *
	HDC hDC;    //�豸������h��������handle
	BITMAPINFOHEADER bi;

	//ע�ᴰ����
	register_window_class();
	
	//��������
	if(window!=nullptr)
	window->h_window = CreateWindow(("window"), title,
							WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
	assert(window->h_window != NULL);

	//��ʼ��λͼͷ��ʽ
	init_bm_header(bi, width, height);

	//��ü�����DC
	hDC = GetDC(window->h_window);  
	window->mem_dc = CreateCompatibleDC(hDC);
	ReleaseDC(window->h_window, hDC);

	//����λͼ
	window->bm_dib = CreateDIBSection(window->mem_dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &ptr, 0, 0); //�����豸�޹ؾ��
	assert(window->bm_dib !=NULL);

	window->bm_old = (HBITMAP)SelectObject(window->mem_dc, window->bm_dib);//���´�����λͼ���д��mem_dc
	window->window_fb = (unsigned char*)ptr;
	window->text_height = text_height;
	window->text_width = text_width;
	window->width = width;
	window->height = height;


	AdjustWindowRect(&rect, GetWindowLong(window->h_window, GWL_STYLE), 0);//�������ڴ�С
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2; // GetSystemMetrics(SM_CXSCREEN)��ȡ����Ļ�ķ�Ƭ��
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2; // ���������λ��
	if (sy < 0) sy = 0;

	SetWindowPos(window->h_window, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(window->h_window);
	ShowWindow(window->h_window, SW_NORMAL);

	//��Ϣ����
	msg_dispatch();

	//��ʼ��keys, window_fbȫΪ0
	memset(window->window_fb, 0, (double)width * height * 4);
	memset(window->keys, 0, sizeof(char) * 512);
	return 0;
}

int window_destroy()
{
	if (window->mem_dc)
	{
		if (window->bm_old)
		{
			SelectObject(window->mem_dc, window->bm_old); // д��ԭ����bitmap�������ͷ�DC��
			window->bm_old = NULL;
		}
		DeleteDC(window->mem_dc);
		window->mem_dc = NULL;
	}
	if (window->bm_dib) 
	{
		DeleteObject(window->bm_dib);
		window->bm_dib = NULL;
	}
	if (window->h_window)
	{
		CloseWindow(window->h_window);
		window->h_window = NULL;
	}

	free(window);
	return 0;
}

void msg_dispatch() 
{
	MSG msg;
	while (1) 
	{
		// Peek��������Get��������PM_NOREMOVE��ʾ�������Ϣ������������������Get����
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break; //û��Ϣ���ȷ������Ϣ����Get
		if (!GetMessage(&msg, NULL, 0, 0)) break;

		TranslateMessage(&msg);	 //ת����Ϣ ���ⰴ��->�ַ�
		DispatchMessage(&msg); //������Ϣ���ص�
	}
}

//����һ�����������
void window_draw_text(window_t* window, char* text)
{
#ifdef UNICODE
	wchar_t* wc;
	int len = MultiByteToWideChar(CP_ACP, 0, text, strlen(text), NULL, 0);
	wc = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, text, strlen(text), wc, len);
	wc[len] = '\0';
	SetWindowText(window->text_handle, wc);
	delete[] wc;
#else
	SetWindowText(window->text_handle, text);
#endif // UNICODE
}
static void window_display() 
{
	LOGFONT logfont; //�ı��������
	ZeroMemory(&logfont, sizeof(LOGFONT));
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfHeight = 20; //��������Ĵ�С
	HFONT hFont = CreateFontIndirect(&logfont);

	HDC hDC = GetDC(window->h_window);
	//Ŀ����ε����Ͻ�(x,y), ��ȣ��߶ȣ�������ָ��
	SelectObject(window->mem_dc, hFont);
	SetTextColor(window->mem_dc, RGB(190, 190, 190));
	SetBkColor(window->mem_dc,RGB(80,80,80));
	TextOut(window->mem_dc, 20, 50, "Project Name:MinerSoftRender", strlen("Project Name:MinerSoftRender"));
	TextOut(window->mem_dc, 20, 80, "Author:ChuanMin Zhang", strlen("Author:ChuanMin Zhang"));
	TextOut(window->mem_dc, 20, 20, 
		"�����ת���Ҽ�ƽ�ƣ�ZC�������л���WASDǰ�������ƶ���QR�������ƶ�", 
		strlen("�����ת���Ҽ�ƽ�ƣ�ZC�������л���WASDǰ�������ƶ���QR�������½�"));

	// �Ѽ�����DC�����ݴ���������DC��
	BitBlt(hDC, 0, 0, window->width, window->height, window->mem_dc, 0, 0, SRCCOPY);
	ReleaseDC(window->h_window, hDC);
}

void window_draw(unsigned char* framebuffer)
{
	for (int i = 0; i < window->height; i++)
	{
		for (int j = 0; j < window->width; j++)
		{
			int index = (i * window->width + j) * 4;
			window->window_fb[index]	 = framebuffer[index + 2];
			window->window_fb[index + 1] = framebuffer[index + 1];
			window->window_fb[index + 2] = framebuffer[index];
		}
	}
	window_display();
}

vec2 get_mouse_pos()
{
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(window->h_window, &point); // ����Ļ�ռ�ת�����ڿռ�
	return vec2((float)point.x, (float)point.y);
}

/* misc platform functions */
/*
��δ�����ʵ����һ������ get_native_time()��������ȡ�߾��ȼ�ʱ����High Precision Event Timer��HPET���ĵ�ǰʱ�䡣
HPET ��һ�ּ�ʱ�������и߾��Ⱥ��ȶ��ԣ����ڲ���ʱ������
�ú�����ʵ��ʹ���� Win32 API �е� QueryPerformanceCounter() �� QueryPerformanceFrequency() ������
������������ Windows ϵͳ�ṩ�����ڻ�ȡ��ʱ���ĵ�ǰֵ��Ƶ�ʵĺ�����
���� QueryPerformanceCounter() ���ؼ�ʱ���ĵ�ǰֵ��QueryPerformanceFrequency() ���ؼ�ʱ����Ƶ�ʡ�
*/
static double get_native_time(void)//����������void��ͬ��û�в�����ֱ��ɾ��Ҳ���ԡ�
{
	static double period = -1;
	LARGE_INTEGER counter;
	//�ú������Ȼ���һ����̬���� period ��ֵ�Ƿ�С�� 0������ǣ���˵����һ�ε��øú�������Ҫ��ȡ��ʱ����Ƶ�ʡ�
	if (period < 0)
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		period = 1 / (double)frequency.QuadPart;
	}
	//���� QueryPerformanceCounter() ��ȡ��ʱ���ĵ�ǰֵ������ period �Ϳ��Եõ���ǰʱ�䡣��󣬽�������ĵ�ǰʱ�䷵�ء�
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * period;
}

/*
��δ����У�initial�ĳ�ʼֵΪ-1����ζ��������ڵ�һ�α�����ʱ��Ȼ����-1����ô������δ��ʼ���ġ�
�ڵ�һ�ε��� platform_get_time ����֮ǰ���ú����ڲ��� get_native_time �����������ã��Ի�ȡ��һ��ʱ�����
Ȼ�󣬵�ǰʱ�����ȥ��һ��ʱ�����������һ�����ʱ�䣬��ʾ�ӵ�һ��ʱ�����ʼ���ŵ�ʱ�䡣
����һ�ֳ�����ģʽ�����г�ʼ�����Ƴٵ���һ��ʹ��֮ǰ����������ȷ����Դֻ����Ҫʱ������ͳ�ʼ����
*/
float platform_get_time(void) {
	static double initial = -1;
	if (initial < 0) {
		initial = get_native_time();
	}
	return (float)(get_native_time() - initial);
}