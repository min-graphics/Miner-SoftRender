#pragma once
#include "./tgaimage.h"
#include "../shader/shader.h"

typedef struct scene
{
	const char* scene_name;//场景名称
	void (*build_scene)(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);//一个函数指针函数指针其实很好认
	/*
	void build_scene(Model **model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera);
	之后在名称上取值，然后加括号。
	void (*build_scene)(Model **model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera);

	可以直接用函数名来指向 
	*/
	char text[500];
	char* get_text();
	void tick(float deltatime);
}scene_t;

TGAImage* texture_from_file(const char* file_name);
void load_ibl_map(payload_t& p, const char* env_path);

void build_fuhua_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_yayi_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_qiyana_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_xier_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_helmet_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_gun_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_yongzhuangfuhua_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_sanyueqi_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_yinlang_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_mona_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_difa_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
void build_crab_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);
