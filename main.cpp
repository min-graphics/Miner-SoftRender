#define _CRT_SECURE_NO_WARNINGS
#include <ctime>
#include <iostream>
#include <random>
#include <chrono>

#include "./core/macro.h"
#include "./core/tgaimage.h"
#include "./core/model.h"
#include "./core/camera.h"
#include "./core/pipeline.h"
#include "./core/sample.h"
#include "./core/scene.h"
#include "./platform/win32.h"
#include "./shader/shader.h"

using namespace std;

//下面的三个向量用来拼起相机矩阵,可以在每个场景中针对不同的模型进行微调
const vec3 Eye(0, 1, 2);
//const vec3 EyePBR(0, 1, 5);
const vec3 Up(0, 1, 0);
const vec3 Target(0, 1, 0);

// 创建相机,根据上面的三个向量
Camera camera(Eye, Target, Up, (float)(WINDOW_WIDTH) / WINDOW_HEIGHT);//创建相机

std::mt19937 rng(std::random_device{}());
static int scene_index = rng() % SCENENUM;
//static int scene_index = 3;

//创建场景，还可以不断往里添加新的，只要有模型，改几个数值就可以了
const scene_t Scenes[] =											//创建场景
{
	{"fuhua",build_fuhua_scene},	//场景里面包含了模型
	{"qiyana",build_qiyana_scene},
	{"yayi",build_yayi_scene},
	{"xier",build_xier_scene},
	{"helmet",build_helmet_scene},
	{"gun",build_gun_scene},
	{"yongzhuangfuhua",build_yongzhuangfuhua_scene},
	{"sanyueqi",build_sanyueqi_scene},
	{"yinlang",build_yinlang_scene},
	{"mona",build_mona_scene},
	{"crab",build_crab_scene},
	{"difa",build_difa_scene},
};
/*
scene_t scene_info;

scene_t load_scene(int scene_index)
{
	if (scene_info.scene_name)
	{
		delete scene_info.scene_name;
	}
	scene_t ans;
	switch(scene_index)
	{
	case 0:
		ans.scene_name = "1.fuhua";
		ans.build_scene = build_fuhua_scene;
		break;
	case 1:
		ans.scene_name = "2.qiyana";
		ans.build_scene = build_qiyana_scene;
		break;
	case 2:
		ans.scene_name = "3.yayi";
		ans.build_scene = build_yayi_scene;
		break;
	case 3:
		ans.scene_name = "xier";
		ans.build_scene = build_xier_scene;
		break;
	case 4:
		ans.scene_name = "helmet";
		ans.build_scene = build_helmet_scene;
		break;
	case 5:
		ans.scene_name = "gun";
		ans.build_scene = build_gun_scene;
		break;

	default:
		ans.scene_name = "xier";
		ans.build_scene = build_xier_scene;
		break;
	}
	return ans;
}
*/
void clear_zbuffer(int width, int height, float* zbuffer)
{
	for (int i = 0; i < width * height; i++)
		zbuffer[i] = 100000;//设为100000，也就是无穷远了。用1维来表示二维的，只要知道横纵坐标来个映射就好了。
}

//清空framebuffer，同时也可以在这里设置背景颜色、、、、
void clear_framebuffer(int width, int height, unsigned char* framebuffer)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			//这里就是通过二维坐标(x，y)定义到一维数组的相关位置。
			int index = (i * width + j) * 4;

			//这里的数值是自定义的
			framebuffer[index] = 100;
			framebuffer[index + 1] = 100;
			framebuffer[index + 2] = 100;
		}
	}
}

void update_matrix(Camera& camera, mat4 view_mat, mat4 perspective_mat, IShader* shader_model, IShader* shader_skybox)
{
	view_mat = mat4_lookat(camera.eye, camera.target, camera.up);
	mat4 mvp = perspective_mat * view_mat;
	shader_model->payload.camera_view_matrix = view_mat;
	shader_model->payload.mvp_matrix = mvp;

	//使用了天空盒，环境贴图
	if (shader_skybox != nullptr)
	{
		mat4 view_skybox = view_mat;
		view_skybox[0][3] = 0;
		view_skybox[1][3] = 0;
		view_skybox[2][3] = 0;
		shader_skybox->payload.camera_view_matrix = view_skybox;
		shader_skybox->payload.mvp_matrix = perspective_mat * view_skybox;
	}
}

void update_text(char* screen_text, int text_size, int show_num_frames, int show_avg_millis)
{
	snprintf(screen_text, text_size, "");

	char line[50] = "";

	snprintf(line, 50, "fps: %3d, avg: %3d ms\n\n", show_num_frames, show_avg_millis);
	strcat(screen_text, line);

	snprintf(line, 50, "scene: %s\n", Scenes[scene_index].scene_name);
	strcat(screen_text, line);

	snprintf(line, 50, "press key [W] or [S] to switch scene\n\n");
	strcat(screen_text, line);
	//zheli不太一样
	strcat(screen_text, Scenes[scene_index].text);

	window_draw_text(window, screen_text);
}

int main()
{
	// initialization
	// --------------
	// malloc memory for zbuffer and framebuffer
	// 
	//初始化，给zbuffer和缓冲buffer分配内存，并置0
	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;

	//虽然使用malloc申请的是一维数组，而zbuffer和framebuffer应该是二维数组，但是没关系。我们依然可以使用二维数组(x,y)的方式来映射求取定位。
	//比如int index = (y * width + x) * 4; 9.
	float* zbuffer = (float*)malloc(sizeof(float) * width * height);
	unsigned char* framebuffer = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 4);//这个申请的内存大小*4并不是由于unsigned int的问题，而是由于要有4个通道来记录RGBA
	//给申请的framebuffer 使用memset进行初始化
	if (framebuffer != nullptr)memset(framebuffer, 0, sizeof(unsigned char) * width * height * 4);

	// 设置mvp矩阵
	mat4 model_mat = mat4::identity();//单位阵，意味着按照模型的建模情况摆放位置了。//M矩阵
	mat4 view_mat = mat4_lookat(camera.eye, camera.target, camera.up);			//V矩阵
	mat4 perspective_mat = mat4_perspective(60, (float)(width) / height, -0.1, -10000);	//P矩阵

	// initialize window
	//初始化一个窗口
	window_init(width, height, "Miner SoftRender");

	// initialize models and shaders by builidng a scene
	//通过建立一个场景来初始化模型和着色器
	//srand((unsigned int)time(NULL));
	//声明为static关键字，使得全局，不会被刷新

	int model_num = 0;
	Model* model[MAX_MODEL_NUM];
	IShader* shader_model = nullptr;
	IShader* shader_skybox = nullptr;
	Scenes[scene_index].build_scene(model, model_num, &shader_model, &shader_skybox, perspective_mat, &camera);

	/*
	scene_info = load_scene(scene_index);
	scene_info.build_scene(model, model_num, &shader_model, &shader_skybox, perspective_mat, &camera);*/

	// render loop
	// -----------
	//渲染循环
	int num_frames = 0;//帧数
	float print_time = platform_get_time();//打印时间
	float prev_time = print_time;

	char screen_text[TEXT_SIZE];
	int show_num_frames = 0;
	int show_avg_millis = 0;
	float refresh_screen_text_timer = 0;
	snprintf(screen_text, TEXT_SIZE, "fps:- -,avg:- - ms\n");

	while (!window->is_close)
	{
		float curr_time = platform_get_time();//获取当前时间
		float delta = 0.005;

		// clear buffer清除上一帧的buffer
		clear_framebuffer(width, height, framebuffer);
		clear_zbuffer(width, height, zbuffer);

		// handle events and update view, perspective matrix
		//处理相机时间，更新矩阵
		handle_events(camera);
		update_matrix(camera, view_mat, perspective_mat, shader_model, shader_skybox);

		if (window->keys['Z'])
		{
			//scene_info = load_scene((scene_index - 1 + SCENENUM) % SCENENUM);
			////scene_info.build_scene(model, model_num, &shader_model, &shader_skybox, perspective_mat, &camera);
			scene_index = (scene_index - 1 + SCENENUM) % SCENENUM;
			Scenes[scene_index].build_scene(model, model_num, &shader_model, &shader_skybox, perspective_mat, &camera);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		if (window->keys['C'])
		{
			/*scene_info = load_scene((scene_index + 1 + SCENENUM) % SCENENUM);
			scene_info.build_scene(model, model_num, &shader_model, &shader_skybox, perspective_mat, &camera);*/
			scene_index = (scene_index + 1 + SCENENUM) % SCENENUM;
			Scenes[scene_index].build_scene(model, model_num, &shader_model, &shader_skybox, perspective_mat, &camera);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

		}

		//draw models
		//进行模型绘制
		for (int m = 0; m < model_num; m++)
		{
			// assign model data to shader
			shader_model->payload.model = model[m];
			if (shader_skybox != NULL) shader_skybox->payload.model = model[m];

			// select current shader according model type
			IShader* shader;
			if (model[m]->is_skybox)
				shader = shader_skybox;
			else
				shader = shader_model;

			for (int i = 0; i < model[m]->nfaces(); i++)
			{
				draw_triangles(framebuffer, zbuffer, *shader, i);
			}
		}

		// calculate and display FPS
		num_frames += 1;
		if (curr_time - print_time >= 1) {
			int sum_millis = (int)((curr_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;

			update_text(screen_text, TEXT_SIZE, show_num_frames, show_avg_millis);

			printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);

			num_frames = 0;
			print_time = curr_time;
		}

		// reset mouse information
		window->mouse_info.wheel_delta = 0;
		window->mouse_info.orbit_delta = vec2(0, 0);
		window->mouse_info.fv_delta = vec2(0, 0);

		// send framebuffer to window 
		window_draw(framebuffer);
		msg_dispatch();
	}

	// free memory，清空缓存，释放空间，能删的都删了，能卸的都卸了，省的内存泄露。
	for (int i = 0; i < model_num; i++)
		if (model[i] != NULL)  delete model[i];
	if (shader_model != NULL)  delete shader_model;
	if (shader_skybox != NULL) delete shader_skybox;

	std::free(zbuffer);
	std::free(framebuffer);

	window_destroy();

	std::system("pause");
	return 0;
}