#pragma once
#include "../core/macro.h"
#include "../core/maths.h"
#include "../core/model.h"
#include "../core/camera.h"

struct light
{
	vec3 pos;
	vec3 intensity;
};

typedef struct cubemap 
{
	TGAImage* faces[6];
}cubemap_t;

typedef struct iblmap 
{
	int mip_levels;
	cubemap_t* irradiance_map;//������diffuseʱ
	cubemap_t* prefilter_maps[15];//���淴��ʱ��Ԥ�˲�������ͼspecular
	TGAImage* brdf_lut;
}iblmap_t;

//��������˹��գ��任���󣬶������ԣ���βü���PBR��IBL����Ҫʹ�õ����Ա��������ڵ�vs��fs֮���һ�����������������á�
typedef struct payload
{
	//����Ǵ�ͳ�����ϵ�mvp�任������˻�
	mat4 model_matrix;//ģ�;���
	mat4 camera_view_matrix;//��ͼ�任����
	mat4 camera_perp_matrix;
	mat4 mvp_matrix;

	//��Դ��ͼ�任����͹�ԴͶӰ�任���󣬿�����������shadow map
	//light_matrix for shadow mapping, (to do)
	mat4 light_view_matrix;
	mat4 light_perp_matrix;

	Camera* camera = nullptr;
	Model* model = nullptr;

	//vertex attribute
	//��������
	vec3 normal_attri[3];
	vec2 uv_attri[3];
	vec3 worldcoord_attri[3];
	vec4 clipcoord_attri[3];

	//Ϊ����βü�
	//for homogeneous clipping
	vec3 in_normal[MAX_VERTEX];
	vec2 in_uv[MAX_VERTEX];
	vec3 in_worldcoord[MAX_VERTEX];
	vec4 in_clipcoord[MAX_VERTEX];
	vec3 out_normal[MAX_VERTEX];
	vec2 out_uv[MAX_VERTEX];
	vec3 out_worldcoord[MAX_VERTEX];
	vec4 out_clipcoord[MAX_VERTEX];

	//for image-based lighting
	iblmap_t* iblmap = nullptr;

	//Ϊ��PBR��ɫ
	bool isPBR = false;

}payload_t;

class IShader
{
public:
	payload_t payload;//��shader�ж�����һ��payload
	virtual void vertex_shader(int nfaces, int nvertex) {}
	virtual vec3 fragment_shader(float alpha, float beta, float gamma) { return vec3(0, 0, 0); }
};

class PhongShader :public IShader
{
public:
	void vertex_shader(int nfaces, int nvertex);
	vec3 fragment_shader(float alpha, float beta, float gamma);
};

class PBRShader :public IShader
{
public:
	PBRShader()
	{
		this->payload.isPBR = true;
	}
	void vertex_shader(int nfaces, int nvertex);
	vec3 fragment_shader(float alpha, float beta, float gamma);
	vec3 direct_fragment_shader(float alpha, float beta, float gamma);
};

class SkyboxShader :public IShader
{
public:
	void vertex_shader(int nfaces, int nvertex);
	vec3 fragment_shader(float alpha, float beta, float gamma);
};
