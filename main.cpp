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

//�����������������ƴ���������,������ÿ����������Բ�ͬ��ģ�ͽ���΢��
const vec3 Eye(0, 1, 2);
//const vec3 EyePBR(0, 1, 5);
const vec3 Up(0, 1, 0);
const vec3 Target(0, 1, 0);

// �������,�����������������
Camera camera(Eye, Target, Up, (float)(WINDOW_WIDTH) / WINDOW_HEIGHT);//�������

std::mt19937 rng(std::random_device{}());
static int scene_index = rng() % SCENENUM;
//static int scene_index = 3;

//���������������Բ�����������µģ�ֻҪ��ģ�ͣ��ļ�����ֵ�Ϳ�����
const scene_t Scenes[] =											//��������
{
	{"fuhua",build_fuhua_scene},	//�������������ģ��
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
		zbuffer[i] = 100000;//��Ϊ100000��Ҳ��������Զ�ˡ���1ά����ʾ��ά�ģ�ֻҪ֪��������������ӳ��ͺ��ˡ�
}

//���framebuffer��ͬʱҲ�������������ñ�����ɫ��������
void clear_framebuffer(int width, int height, unsigned char* framebuffer)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			//�������ͨ����ά����(x��y)���嵽һά��������λ�á�
			int index = (i * width + j) * 4;

			//�������ֵ���Զ����
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

	//ʹ������պУ�������ͼ
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
	//zheli��̫һ��
	strcat(screen_text, Scenes[scene_index].text);

	window_draw_text(window, screen_text);
}

int main()
{
	// initialization
	// --------------
	// malloc memory for zbuffer and framebuffer
	// 
	//��ʼ������zbuffer�ͻ���buffer�����ڴ棬����0
	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;

	//��Ȼʹ��malloc�������һά���飬��zbuffer��framebufferӦ���Ƕ�ά���飬����û��ϵ��������Ȼ����ʹ�ö�ά����(x,y)�ķ�ʽ��ӳ����ȡ��λ��
	//����int index = (y * width + x) * 4; 9.
	float* zbuffer = (float*)malloc(sizeof(float) * width * height);
	unsigned char* framebuffer = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 4);//���������ڴ��С*4����������unsigned int�����⣬��������Ҫ��4��ͨ������¼RGBA
	//�������framebuffer ʹ��memset���г�ʼ��
	if (framebuffer != nullptr)memset(framebuffer, 0, sizeof(unsigned char) * width * height * 4);

	// ����mvp����
	mat4 model_mat = mat4::identity();//��λ����ζ�Ű���ģ�͵Ľ�ģ����ڷ�λ���ˡ�//M����
	mat4 view_mat = mat4_lookat(camera.eye, camera.target, camera.up);			//V����
	mat4 perspective_mat = mat4_perspective(60, (float)(width) / height, -0.1, -10000);	//P����

	// initialize window
	//��ʼ��һ������
	window_init(width, height, "Miner SoftRender");

	// initialize models and shaders by builidng a scene
	//ͨ������һ����������ʼ��ģ�ͺ���ɫ��
	//srand((unsigned int)time(NULL));
	//����Ϊstatic�ؼ��֣�ʹ��ȫ�֣����ᱻˢ��

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
	//��Ⱦѭ��
	int num_frames = 0;//֡��
	float print_time = platform_get_time();//��ӡʱ��
	float prev_time = print_time;

	char screen_text[TEXT_SIZE];
	int show_num_frames = 0;
	int show_avg_millis = 0;
	float refresh_screen_text_timer = 0;
	snprintf(screen_text, TEXT_SIZE, "fps:- -,avg:- - ms\n");

	while (!window->is_close)
	{
		float curr_time = platform_get_time();//��ȡ��ǰʱ��
		float delta = 0.005;

		// clear buffer�����һ֡��buffer
		clear_framebuffer(width, height, framebuffer);
		clear_zbuffer(width, height, zbuffer);

		// handle events and update view, perspective matrix
		//�������ʱ�䣬���¾���
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
		//����ģ�ͻ���
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

	// free memory����ջ��棬�ͷſռ䣬��ɾ�Ķ�ɾ�ˣ���ж�Ķ�ж�ˣ�ʡ���ڴ�й¶��
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