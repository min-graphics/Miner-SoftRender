#pragma once
#include "./tgaimage.h"
#include "../shader/shader.h"

typedef struct scene
{
	const char* scene_name;//��������
	void (*build_scene)(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera);//һ������ָ�뺯��ָ����ʵ�ܺ���
	/*
	void build_scene(Model **model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera);
	֮����������ȡֵ��Ȼ������š�
	void (*build_scene)(Model **model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera);

	����ֱ���ú�������ָ�� 
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
