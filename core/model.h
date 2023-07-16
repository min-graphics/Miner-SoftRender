#pragma once
#include <string>
#include <vector>
#include <io.h> 
#include <iostream>
#include <fstream>
#include <sstream>

#include "./maths.h"
#include "./tgaimage.h"

typedef struct cubemap cubemap_t; // forward declaration

class Model
{
private:
	std::vector<vec3> verts;//顶点
	std::vector<std::vector<int> > faces; //面
	std::vector<vec3> norms;//法线
	std::vector<vec2> uvs;//纹理数据

public:
	vec3 vert(int i);//返回第i个顶点的坐标。
	vec3 vert(int iface, int nthvert);//返回给定面iface上的第nthvert个顶点的坐标。
	int nverts();//返回模型顶点数
	int nfaces();//返回模型面数
	vec3 normal(int iface, int nthvert);//返回给定面iface上的第nthvert个顶点的法向量。
	vec2 uv(int iface, int nthvert);//返回给定面iface上的第nthvert个顶点的纹理坐标。

	std::vector<int> face(int idx);//这个函数是用来获取模型中第idx个面的顶点索引的，返回值是一个存储int类型的vector，其中包含了这个面的三个顶点的索引值。这个函数可以用于在渲染时，从模型中获取每个面的顶点数据，以便将它们传递给图形API进行渲染。

public:
	//skybox
	cubemap_t* environment_map;
	bool is_skybox;

	//map
	bool is_from_mmd;
	TGAImage* diffusemap;//漫反射贴图,决定物体表面基本颜色，它通常包含了物体表面的颜色、纹理、阴影等信息，用于模拟物体表面的基本色调。
	TGAImage* normalmap;//法线贴图,用于模拟物体表面的凹凸不平，可以在不改变物体形状的情况下增加物体的表面细节。
	TGAImage* specularmap;//镜面反射贴图,用于模拟物体表面的高光反射部分，通常用于描述物体表面的镜面反射亮度和大小。
	TGAImage* roughnessmap;//粗糙度贴图,用于模拟物体表面的粗糙程度，通常用于调整物体表面的光泽度。
	TGAImage* metalnessmap;//金属度贴图,用于模拟物体表面的金属属性，通常用于控制物体表面反射的颜色和强度。
	TGAImage* occlusion_map;//遮挡贴图,用于模拟物体表面不同区域之间的遮挡关系，通常用于增加物体表面的阴影效果。
	TGAImage* emision_map;//自发光贴图,用于模拟物体表面的自发光效果，通常用于增加物体表面的光源效果。
	TGAImage* glossiness_map;//光泽度贴图，和roughness是1-的关系

public:
	vec3 diffuse(vec2 uv);	 //返回给定纹理坐标uv处的漫反射颜色。
	vec3 normal(vec2 uv);	 //返回给定纹理坐标uv处的法向量。
	float specular(vec2 uv); //返回给定纹理坐标uv处的高光强度。
	float roughness(vec2 uv);//返回给定纹理坐标uv处的粗糙度。
	float metalness(vec2 uv);//返回给定纹理坐标uv处的金属度。
	float occlusion(vec2 uv);//返回给定纹理坐标uv处的遮挡度。
	vec3 emission(vec2 uv);	 //返回给定纹理坐标uv处的自发光颜色。
	float glossiness(vec2 uv);
	
public:

	//构造函数，用于从文件中加载模型数据，is_skybox判断是否为天空盒，和if_from_mmd，是否来自mmd模型
	Model(const char *filename, bool is_skybox = false, bool is_from_mmd = true);
	~Model();

	void load_cubemap(const char* filename);//加载立方体贴图
	void create_map(const char* filename);//创建映射
	void load_texture(std::string filename, const char* suffix, TGAImage& img);//加载纹理数据
	void load_texture(std::string filename, const char* suffix, TGAImage* img);
};