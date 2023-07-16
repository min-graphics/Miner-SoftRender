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
	cubemap_t* irradiance_map;//漫反射diffuse时
	cubemap_t* prefilter_maps[15];//镜面反射时，预滤波环境贴图specular
	TGAImage* brdf_lut;
}iblmap_t;

//里面包含了光照，变换矩阵，顶点属性，齐次裁剪，PBR，IBL等需要使用的属性变量。用于当vs和fs之间的一个桥梁，起到连接作用。
typedef struct payload
{
	//这就是传统意义上的mvp变换矩阵及其乘积
	mat4 model_matrix;//模型矩阵
	mat4 camera_view_matrix;//视图变换矩阵
	mat4 camera_perp_matrix;
	mat4 mvp_matrix;

	//光源视图变换矩阵和光源投影变换矩阵，可以用于生成shadow map
	//light_matrix for shadow mapping, (to do)
	mat4 light_view_matrix;
	mat4 light_perp_matrix;

	Camera* camera = nullptr;
	Model* model = nullptr;

	//vertex attribute
	//顶点属性
	vec3 normal_attri[3];
	vec2 uv_attri[3];
	vec3 worldcoord_attri[3];
	vec4 clipcoord_attri[3];

	//为了齐次裁剪
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

	//为了PBR着色
	bool isPBR = false;

}payload_t;

class IShader
{
public:
	payload_t payload;//在shader中定义了一个payload
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
