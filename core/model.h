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
	std::vector<vec3> verts;//����
	std::vector<std::vector<int> > faces; //��
	std::vector<vec3> norms;//����
	std::vector<vec2> uvs;//��������

public:
	vec3 vert(int i);//���ص�i����������ꡣ
	vec3 vert(int iface, int nthvert);//���ظ�����iface�ϵĵ�nthvert����������ꡣ
	int nverts();//����ģ�Ͷ�����
	int nfaces();//����ģ������
	vec3 normal(int iface, int nthvert);//���ظ�����iface�ϵĵ�nthvert������ķ�������
	vec2 uv(int iface, int nthvert);//���ظ�����iface�ϵĵ�nthvert��������������ꡣ

	std::vector<int> face(int idx);//���������������ȡģ���е�idx����Ķ��������ģ�����ֵ��һ���洢int���͵�vector�����а������������������������ֵ���������������������Ⱦʱ����ģ���л�ȡÿ����Ķ������ݣ��Ա㽫���Ǵ��ݸ�ͼ��API������Ⱦ��

public:
	//skybox
	cubemap_t* environment_map;
	bool is_skybox;

	//map
	bool is_from_mmd;
	TGAImage* diffusemap;//��������ͼ,����������������ɫ����ͨ������������������ɫ��������Ӱ����Ϣ������ģ���������Ļ���ɫ����
	TGAImage* normalmap;//������ͼ,����ģ���������İ�͹��ƽ�������ڲ��ı�������״���������������ı���ϸ�ڡ�
	TGAImage* specularmap;//���淴����ͼ,����ģ���������ĸ߹ⷴ�䲿�֣�ͨ�����������������ľ��淴�����Ⱥʹ�С��
	TGAImage* roughnessmap;//�ֲڶ���ͼ,����ģ���������Ĵֲڳ̶ȣ�ͨ�����ڵ����������Ĺ���ȡ�
	TGAImage* metalnessmap;//��������ͼ,����ģ���������Ľ������ԣ�ͨ�����ڿ���������淴�����ɫ��ǿ�ȡ�
	TGAImage* occlusion_map;//�ڵ���ͼ,����ģ��������治ͬ����֮����ڵ���ϵ��ͨ��������������������ӰЧ����
	TGAImage* emision_map;//�Է�����ͼ,����ģ�����������Է���Ч����ͨ�����������������Ĺ�ԴЧ����
	TGAImage* glossiness_map;//�������ͼ����roughness��1-�Ĺ�ϵ

public:
	vec3 diffuse(vec2 uv);	 //���ظ�����������uv������������ɫ��
	vec3 normal(vec2 uv);	 //���ظ�����������uv���ķ�������
	float specular(vec2 uv); //���ظ�����������uv���ĸ߹�ǿ�ȡ�
	float roughness(vec2 uv);//���ظ�����������uv���Ĵֲڶȡ�
	float metalness(vec2 uv);//���ظ�����������uv���Ľ����ȡ�
	float occlusion(vec2 uv);//���ظ�����������uv�����ڵ��ȡ�
	vec3 emission(vec2 uv);	 //���ظ�����������uv�����Է�����ɫ��
	float glossiness(vec2 uv);
	
public:

	//���캯�������ڴ��ļ��м���ģ�����ݣ�is_skybox�ж��Ƿ�Ϊ��պУ���if_from_mmd���Ƿ�����mmdģ��
	Model(const char *filename, bool is_skybox = false, bool is_from_mmd = true);
	~Model();

	void load_cubemap(const char* filename);//������������ͼ
	void create_map(const char* filename);//����ӳ��
	void load_texture(std::string filename, const char* suffix, TGAImage& img);//������������
	void load_texture(std::string filename, const char* suffix, TGAImage* img);
};