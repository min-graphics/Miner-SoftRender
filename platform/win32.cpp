#include "./win32.h"

window_t* window = NULL;

/*
hWnd:窗口的句柄，标识了该消息所属的窗口。
msg:表示接受到的窗口消息的标识符(Message ID)
wParam:包含了消息相关的附加信息
lParam:包含了消息相关的附加信息
*/
//在函数返回值和函数名之间可以加入 宏定义等修饰符，以保证正确的函数调用约定和参数传递方式，比如CALLBACK可以被定义为__stdcall或者__cdecl，这取决于编译器的设置
/*
__stdcall是一种标准的调用约定，在调用函数时参数由调用方进行堆栈的清理工作。
__cdecl是默认的调用约定，参数由被调用函数自己进行堆栈的清理工作。
*/
static LRESULT CALLBACK msg_callback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) //回调函数类型
{
	switch (msg) 
	{
		case WM_CLOSE: 
			window->is_close = 1; 
			break;
		case WM_KEYDOWN: 
			window->keys[wParam & 511] = 1; //这个&511是位掩码操作，（511=111111111）用于提取wParam的低9位，将值限制在0到511之间
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
//这段代码使用了预处理指令#ifdef，根据编译时是否定义了宏UNICODE来定义两组常量。
//UNICODE宏定义会告诉编译器使用Unicode编码来编译源代码，而没有定义UNICODE宏定义则默认使用ASCII编码。
//
//如果定义了UNICODE宏定义，则常量WINDOW_CLASS_NAME和WINDOW_ENTRY_NAME被定义为wchar_t类型的字符串，即宽字符类型的字符串，以L开头；
//否则被定义为char类型的字符串，即窄字符类型的字符串。常量WINDOW_CLASS_NAME是窗口类名，常量WINDOW_ENTRY_NAME是窗口的标题名。
//
//这样做的目的是为了在不同编译环境下能够正确地定义窗口类名和窗口标题名，以确保在不同的系统中能够正确地创建窗口。
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
	//初始化结构体
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;						//窗口风格
	wc.lpfnWndProc = (WNDPROC)msg_callback;					//回调函数
	wc.cbClsExtra = 0;										//紧跟在窗口类尾部的一块额外空间，不用则设为0
	wc.cbWndExtra = 0;										//紧跟在窗口实例尾部的一块额外空间，不用则设为0
	wc.hInstance = GetModuleHandle(NULL);					//当前实例句柄
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);				//任务栏图标
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);				//光标样式
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//背景样式
	wc.lpszMenuName = NULL;									//菜单
	wc.lpszClassName = "window";					//该窗口类的名字

	atom = RegisterClass(&wc); //注册窗口类
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
	bi.biHeight = -height;   //从上到下
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
//改
	window->text_handle = CreateWindow(TEXT("static"), TEXT("static"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, text_width, text_height, window->h_window, HMENU(21), GetModuleHandle(NULL), NULL);
	SetTextColor(GetDC(window->text_handle), RGB(255, 0, 0));
//
	RECT rect = { 0, 0, width, height }; //一个矩形范围 左上右下
	int wx, wy, sx, sy;
	LPVOID ptr; //就是void *
	HDC hDC;    //设备环境，h代表句柄，handle
	BITMAPINFOHEADER bi;

	//注册窗口类
	register_window_class();
	
	//创建窗口
	if(window!=nullptr)
	window->h_window = CreateWindow(("window"), title,
							WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
	assert(window->h_window != NULL);

	//初始化位图头格式
	init_bm_header(bi, width, height);

	//获得兼容性DC
	hDC = GetDC(window->h_window);  
	window->mem_dc = CreateCompatibleDC(hDC);
	ReleaseDC(window->h_window, hDC);

	//创建位图
	window->bm_dib = CreateDIBSection(window->mem_dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &ptr, 0, 0); //创建设备无关句柄
	assert(window->bm_dib !=NULL);

	window->bm_old = (HBITMAP)SelectObject(window->mem_dc, window->bm_dib);//把新创建的位图句柄写入mem_dc
	window->window_fb = (unsigned char*)ptr;
	window->text_height = text_height;
	window->text_width = text_width;
	window->width = width;
	window->height = height;


	AdjustWindowRect(&rect, GetWindowLong(window->h_window, GWL_STYLE), 0);//调整窗口大小
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2; // GetSystemMetrics(SM_CXSCREEN)获取你屏幕的分片率
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2; // 计算出中心位置
	if (sy < 0) sy = 0;

	SetWindowPos(window->h_window, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(window->h_window);
	ShowWindow(window->h_window, SW_NORMAL);

	//消息分派
	msg_dispatch();

	//初始化keys, window_fb全为0
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
			SelectObject(window->mem_dc, window->bm_old); // 写入原来的bitmap，才能释放DC！
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
		// Peek不阻塞，Get会阻塞，PM_NOREMOVE表示如果有消息不处理（留给接下来的Get处理）
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break; //没消息就溜，确定有消息再用Get
		if (!GetMessage(&msg, NULL, 0, 0)) break;

		TranslateMessage(&msg);	 //转换消息 虚拟按键->字符
		DispatchMessage(&msg); //传送消息给回调
	}
}

//从另一个借鉴过来的
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
	LOGFONT logfont; //改变输出字体
	ZeroMemory(&logfont, sizeof(LOGFONT));
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfHeight = 20; //设置字体的大小
	HFONT hFont = CreateFontIndirect(&logfont);

	HDC hDC = GetDC(window->h_window);
	//目标矩形的左上角(x,y), 宽度，高度，上下文指针
	SelectObject(window->mem_dc, hFont);
	SetTextColor(window->mem_dc, RGB(190, 190, 190));
	SetBkColor(window->mem_dc,RGB(80,80,80));
	TextOut(window->mem_dc, 20, 50, "Project Name:MinerSoftRender", strlen("Project Name:MinerSoftRender"));
	TextOut(window->mem_dc, 20, 80, "Author:ChuanMin Zhang", strlen("Author:ChuanMin Zhang"));
	TextOut(window->mem_dc, 20, 20, 
		"左键旋转，右键平移，ZC键场景切换，WASD前后左右移动，QR键上下移动", 
		strlen("左键旋转，右键平移，ZC键场景切换，WASD前后左右移动，QR键上升下降"));

	// 把兼容性DC的数据传到真正的DC上
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
	ScreenToClient(window->h_window, &point); // 从屏幕空间转到窗口空间
	return vec2((float)point.x, (float)point.y);
}

/* misc platform functions */
/*
这段代码是实现了一个函数 get_native_time()，用来获取高精度计时器（High Precision Event Timer，HPET）的当前时间。
HPET 是一种计时器，具有高精度和稳定性，用于测量时间间隔。
该函数的实现使用了 Win32 API 中的 QueryPerformanceCounter() 和 QueryPerformanceFrequency() 函数。
这两个函数是 Windows 系统提供的用于获取计时器的当前值和频率的函数，
其中 QueryPerformanceCounter() 返回计时器的当前值，QueryPerformanceFrequency() 返回计时器的频率。
*/
static double get_native_time(void)//函数参数是void等同于没有参数，直接删除也可以。
{
	static double period = -1;
	LARGE_INTEGER counter;
	//该函数首先会检查一个静态变量 period 的值是否小于 0，如果是，则说明第一次调用该函数，需要获取计时器的频率。
	if (period < 0)
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		period = 1 / (double)frequency.QuadPart;
	}
	//调用 QueryPerformanceCounter() 获取计时器的当前值，乘以 period 就可以得到当前时间。最后，将计算出的当前时间返回。
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * period;
}

/*
这段代码中，initial的初始值为-1，意味着如果它在第一次被访问时仍然等于-1，那么它就是未初始化的。
在第一次调用 platform_get_time 函数之前，该函数内部的 get_native_time 函数将被调用，以获取第一个时间戳。
然后，当前时间戳减去第一个时间戳，将返回一个相对时间，表示从第一个时间戳开始流逝的时间。
这是一种常见的模式，其中初始化被推迟到第一次使用之前，这样可以确保资源只在需要时被分配和初始化。
*/
float platform_get_time(void) {
	static double initial = -1;
	if (initial < 0) {
		initial = get_native_time();
	}
	return (float)(get_native_time() - initial);
}