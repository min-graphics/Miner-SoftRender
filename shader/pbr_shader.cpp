#include "./shader.h"
#include "../core/tgaimage.h"
#include "../core/sample.h"
#include <cmath>


/*
D��:���߷ֲ�����
΢ƽ�������е�ggx�ֲ�������
�ڸ÷ֲ������У�һ��΢ƽ��ķ��������ӽ�����֮��ļнǺͷ��������Դ����֮��ļнǹ�ͬ������΢ƽ�淴������ȡ�
ʵ�ֽ��������ϵȱ����΢�۴ֲڶȣ��Լ����ʵʱ��Ⱦ�����еķ������Ч����
*/
static float GGX_distribution(float n_dot_h, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;

	float n_dot_h_2 = n_dot_h * n_dot_h;
	float factor = n_dot_h_2 * (alpha2 - 1) + 1;
	return alpha2 / (PI * factor * factor);
}


//F:������������۲�Ƕ��뷴��ƽ�淽��ļнǶ�����ķ���̶Ȳ�ͬ
//���Է������������΢ƽ�淨��(�����й��׵�΢ƽ��)��۲췽��ļн�
//F:���������Ҫ��������ʾ�۲�ǶȺͷ���֮��Ĺ�ϵ��Ҳ����˵��ͨ�۲�Ƕ��£��������ռ�����Ķ��١�
//������ȷ�ķ��������Ҫ���ǹ��ߵ�S������P������֮������ֵ��ƽ���������������Ǽ򵥵�ʹ��ʩ���˽��ơ�
static vec3 fresenlschlick(float h_dot_v, vec3& f0)
{
	return f0 + (vec3(1.0, 1.0, 1.0) - f0) * pow(1 - h_dot_v, 5.0);
}
static vec3 fresenlschlick_roughness(float h_dot_v, vec3& f0, float roughness)
{
	float r1 = 1.0f - roughness;
	if (r1 < f0[0])
		r1 = f0[0];
	return f0 + (vec3(r1, r1, r1) - f0) * pow(1 - h_dot_v, 5.0f);
}


//G:�����ڱ����Ҫ��ʾ΢�����ϸСƽ������ڵ������һ����ʷ��˹��������ʾ���ֱ����������߱��ڵ�����������߱��ڵ������������Ҫע�������k������ֱ�ӹ��ջ���IBL��ӹ��գ����������ض���
static float SchlickGGX_geometry(float n_dot_v, float roughness)
{
	float r = (1 + roughness);
	float k = (double)r * r / 8.0;

	return n_dot_v / (n_dot_v*(1 - k) + k);
}
static float SchlickGGX_geometry_ibl(float n_dot_v, float roughness)
{
	float k = (double)roughness * roughness / 2.0;

	return n_dot_v / (n_dot_v*(1 - k) + k);
}
//G:ʷ��˹���̣������ڵ�����ˣ������ڵ�������������۱����Ӱ�죬�����Ǻ�۷���
static float geometry_Smith(float n_dot_v, float n_dot_l, float roughness)
{
	float g1 = SchlickGGX_geometry(n_dot_v, roughness);
	float g2 = SchlickGGX_geometry(n_dot_l, roughness);

	return g1 * g2;
}


/*
�������Ҳ����������ɫ��ӳ��ġ�������hdrת��ldr,���������ʵ����Reinhard mapping��һ���֡�

�������ʵ����һ����Ϊ "ACES" (Academy Color Encoding System) ��ɫ�ʹ���ϵͳ�е�ɫ��ӳ���㷨��
���㷨ͨ��������ֵ (ͨ����ɫ��ֵ) ӳ�䵽�µ����ֵ���ı���ɫ����ۡ�

�ں����У�����һ��������ֵ value���ú���ͨ������ֵ����Ԥ�����һЩ���� (a��b��c��d��e)��������һЩ�򵥵���ѧ����������ӳ����ֵ��
���ս��ͨ������ float_clamp ������ȷ������ 0 �� 1 ֮�䡣
����ɫ��ӳ�似��ͨ�����ڽ�һ��ɫ�ʿռ��е���ɫֵӳ�䵽��һ��ɫ�ʿռ��е���ɫֵ���������ڽ�����ɫУ���͵�������ʵ�ָ��õ�ͼ����������õ��Ӿ�Ч����
*/
static float float_aces(float value)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	value = (value * (a * value + b)) / (value * (c * value + d) + e);
	return float_clamp(value, 0, 1);
}


//Reinhardɫ��ӳ�䷽���������vec3��HDR�������color��LDR��ɫ��Ŀ����ʹHDRͼ����LDR��ʾ�豸�ϸ��õĳ��ֳ�����
//��LDR�У�ɫ�ʱ���������0-1֮�䱻��ʾ�������ᵼ��ʧȥ�ܶ��ϸ�ڣ�������ݺ�̫�������ܶ�����һƬ�ס�̫���ĵط���̫���ĵط�ϸ�ڶ�ʧ��HDR����ͨ��ɫ��ӳ�䣬��LDR��ת�뵽�߶�̬��Χ�У�
//�ܱ����������ʾϸ�ڡ�
static vec3 Reinhard_mapping(vec3& color)
{
	for (int i = 0; i < 3; i++)
	{
		color[i] = float_aces(color[i]);	
		//color[i] = color[i] / (color[i] + 0.5);
		
		color[i] = pow(color[i], 1.0 / 2.2);
	}
	return color;
}


//���㷨����ͼ��������ķ�������
//���ܵĲ����ֱ��ǣ�ԭʼ���������������������飬�����������飬��ǰƬ�ε����������Լ�������ͼ��TGAͼ��
/*
������ͼ�еķ����������������߿ռ��У������߿ռ��У�������Զָ����Z����
��������ڵ��������εı��زο���ܡ�
����������ͼ�����ı��ؿռ䣻���Ƕ�������Ϊָ����z�����������ձ任��ʲô����
ʹ��һ���ض��ľ������Ǿ��ܽ�����/���߿ռ��еķ�������ת���������ͼ�ռ��£�ʹ����ת�����յ���ͼ����ķ���
֮�����ǰ����߿ռ��Z����ͱ���ķ��߷�����뼴�ɡ�
���־������TBN������������ĸ�ֱ����tangent��bitangent��normal���������ǽ���������������������
Ҫ��������һ�������߿ռ�ת��Ϊ��ͬ�ռ�ı������������Ҫ�����໥��ֱ��������������һ������ķ�����ͼ�����ڣ��ϡ��ҡ�ǰ�����������������̳����������ơ�

��������ߺ͸����߲�������������ô���ס���ͼ�п��Կ���������ͼ�����ߺ͸��������������������������롣
���Ǿ����õ�������Լ���ÿ����������ߺ͸����ߵġ���Ҫ�õ�һЩ��ѧ���ܵõ����ǣ�
*/
static vec3 cal_normal(vec3& normal, vec3* world_coords, const vec2* uvs, const vec2& uv, TGAImage* normal_map)
{
	//learnOpenGL�ｲ����������ǲ���������ͼ������������仹ԭ�ɷ���ֵ��
	//�ӷ�����ͼ�в����õ�һ����������
	vec3 sample = texture_sample(uv, normal_map);
	//modify the range 0 ~ 1 to -1 ~ +1
	//�Բ����õ��ķ����������з�Χӳ�䣬�����0��1�ķ�Ϊת��Ϊ-1��+1�ķ�Χ��
	sample = vec3(sample[0] * 2 - 1, sample[1] * 2 - 1, sample[2] * 2 - 1);

	//�����Ҫ����TBN����

	//calculate the difference in UV coordinate
	//��������������Ĳ��죬�����������ߵĲ�ֵ��x1��y1��x2��y2.������Щ��ֵ������һ������ʽֵ(det)��
	float x1 = uvs[1][0] - uvs[0][0];
	float y1 = uvs[1][1] - uvs[0][1];
	float x2 = uvs[2][0] - uvs[0][0];
	float y2 = uvs[2][1] - uvs[0][1];

	float det = (x1 * y2 - x2 * y1);

	//calculate the difference in world pos
	//��������������֮��Ĳ��죬ͨ������������֮��Ĳ�ֵ(e1��e2)
	vec3 e1 = world_coords[1] - world_coords[0];
	vec3 e2 = world_coords[2] - world_coords[0];

	//calculate tangent-axis and bitangent-axis
	//������������tangent-axis�͸�������bitangent-axis
	vec3 t = e1 * y2 + e2 * (-y1);
	vec3 b = e1 * (-x2) + e2 * x1;
	t /= det;//ͨ������֮ǰ���������ʽֵ����һ��
	b /= det;

	//ʩ��������������֤3���������������ģ��Ƴ���Ŀǰ����صķ�����
	//Schmidt orthogonalization
	normal = unit_vector(normal);
	t = unit_vector(t - dot(t, normal)*normal);
	b = unit_vector(b - dot(b, normal)*normal - dot(b, t)*t);

	//ʹ�������ᡢ���������ԭʼ����������Ȩ����Ϸ����������õ����յ��·������������·���������Ϊ�����ķ���ֵ��
	vec3 normal_new = t * sample[0] + b * sample[1] + normal * sample[2];
	return normal_new;
}


void PBRShader::vertex_shader(int nfaces, int nvertex)
{
	vec4 temp_vert = to_vec4(payload.model->vert(nfaces, nvertex), 1.0f);
	vec4 temp_normal = to_vec4(payload.model->normal(nfaces, nvertex), 1.0f);

	payload.uv_attri[nvertex] = payload.model->uv(nfaces, nvertex);
	payload.in_uv[nvertex] = payload.uv_attri[nvertex];
	payload.clipcoord_attri[nvertex] = payload.mvp_matrix * temp_vert;
	payload.in_clipcoord[nvertex] = payload.clipcoord_attri[nvertex];

	//only model matrix can change normal vector//ֻ��ģ�;�����Ըı䷨������
	for (int i = 0; i < 3; i++)
	{
		payload.worldcoord_attri[nvertex][i]     = temp_vert[i];
		payload.in_worldcoord[nvertex][i]        = temp_vert[i];
		payload.normal_attri[nvertex][i]         = temp_normal[i];
		payload.in_normal[nvertex][i]            = temp_normal[i];
	}
}

//��ӽ�ȥ��
vec3 PBRShader::direct_fragment_shader(float alpha, float beta, float gamma)
{
	vec3 CookTorrance_brdf;//�����洢cooktorrance brdf�Ľ��

	vec3 light_pos = vec3(2, 1.5, 5);//��Դ��λ��
	vec3 radiance = vec3(3,3,3);//��Դ�ķ��նȣ�Ҳ���ǹ�Դ����ɫ

	//for reading easily
	vec4* clip_coords = payload.clipcoord_attri;//�ü��ռ������
	vec3* world_coords = payload.worldcoord_attri;//��������
	vec3* normals = payload.normal_attri;//��������
	vec2* uvs = payload.uv_attri;//uv��������

	//interpolate attribute,�����Խ��в�ֵ
	float Z = 1.0 / ((double)alpha / clip_coords[0].w() + beta / clip_coords[1].w() + gamma / clip_coords[2].w());
	vec3 normal = (alpha * normals[0] / clip_coords[0].w() + beta * normals[1] / clip_coords[1].w() +
		gamma * normals[2] / clip_coords[2].w()) * Z;
	vec2 uv = (alpha*uvs[0] / clip_coords[0].w() + beta * uvs[1] / clip_coords[1].w() +
		gamma * uvs[2] / clip_coords[2].w()) * Z;
	vec3 worldpos = (alpha*world_coords[0] / clip_coords[0].w() + beta * world_coords[1] / clip_coords[1].w() +
		gamma * world_coords[2] / clip_coords[2].w()) * Z;

	vec3 l = unit_vector(light_pos - worldpos);//��Դ����
	vec3 n = unit_vector(normal);//������
	vec3 v = unit_vector(payload.camera->eye - worldpos);//��������
	vec3 h = unit_vector(l + v);//�������

	//��Сֵ��0��Ӧ���ڰ����ϻ��֣�����Ӧ���ǻ�����
	float n_dot_l = float_max(dot(n, l), 0);
	
	vec3 color(0,0,0);
	
	if (n_dot_l>0){
		float n_dot_v = float_max(dot(n, v), 0);
		float n_dot_h = float_max(dot(n, h), 0);
		float h_dot_v = float_max(dot(h, v), 0);

		float roughness = payload.model->roughness(uv);
		float metalness = payload.model->metalness(uv);

		//roughness = 0.2;
		//metalness = 0.8;
		float NDF = GGX_distribution(n_dot_h, roughness);
		float G = geometry_Smith(n_dot_v, n_dot_l, roughness);

		//get albedo
		vec3 albedo = payload.model->diffuse(uv);
		vec3 temp = vec3(0.04, 0.04, 0.04);
		vec3 f0 = vec3_lerp(temp, albedo, metalness);

		vec3 F = fresenlschlick(h_dot_v, f0);
		vec3 kD = (vec3(1.0, 1.0, 1.0) - F)*(1 - (double)metalness);

		CookTorrance_brdf = (double)NDF * G * F / (4.0*n_dot_l*n_dot_v + 0.0001);

		vec3 Lo = (kD * albedo/PI + CookTorrance_brdf) * radiance * n_dot_l;
		//std::cout << Lo << std::endl;
		vec3 ambient = 0.05 * albedo;
		color = Lo + ambient;
		

		//vec3 temp2 = (color + vec3(1.0, 1.0, 1.0));
		//for (int i = 0; i < 3; i++)
		//{
			// color[i] = color[i] / temp2[i];
			// color[i] = pow(color[i], 1.0 / 2.2);
		//}
	}

	Reinhard_mapping(color);
	//color = pow(color, vec3((float)1.0 / 2.2, (float)1.0 / 2.2, (float)1.0 / 2.2));
	return color * 255.f;
}


//ibl_fragment_shader
//�������������������£������������в�ֵ��
vec3 PBRShader::fragment_shader(float alpha, float beta, float gamma)
{
	vec3 CookTorrance_brdf;//���ڴ洢cook-torrance brdf�Ľ��

	vec3 light_pos = vec3(2, 1.5, 5);//��ʾ��Դ��λ��
	vec3 radiance = vec3(3, 3, 3);//��ʾ��Դ�ķ���ȣ�Ҳ���ǹ�Դ����ɫ

	//for reading easily����Ҫ����Щ�������ԣ�Ҳ���Ƕ�����ɫ���������ģ�������һЩָ�򶥵����Ե�ָ�롣
	vec4* clip_coords = payload.clipcoord_attri;//ָ��ü��ռ�����
	vec3* world_coords = payload.worldcoord_attri;//ָ����������
	vec3* normals = payload.normal_attri;//ָ��������
	vec2* uvs = payload.uv_attri;//ָ����������

	//interpolate attribute
	float Z = 1.0 / (alpha / (double)clip_coords[0].w() + beta / clip_coords[1].w() + gamma / clip_coords[2].w());//ʹ����������Բü��ռ�����ֵ���м�Ȩƽ�����õ�Ƭ�ε����ֵ��
	
	//�Է����������������꣬����������в�ֵ
	vec3 normal = (alpha * normals[0] / clip_coords[0].w() + beta * normals[1] / clip_coords[1].w() +
		gamma * normals[2] / clip_coords[2].w()) * Z;
	vec2 uv = (alpha * uvs[0] / clip_coords[0].w() + beta * uvs[1] / clip_coords[1].w() +
		gamma * uvs[2] / clip_coords[2].w()) * Z;
	vec3 worldpos = (alpha * world_coords[0] / clip_coords[0].w() + beta * world_coords[1] / clip_coords[1].w() +
		gamma * world_coords[2] / clip_coords[2].w()) * Z;

	//�ӷ�����ͼ�в����õ���������
	if (payload.model->normalmap)
	{
		normal = cal_normal(normal, world_coords, uvs, uv, payload.model->normalmap);
	}

	//��������
	vec3 n = unit_vector(normal);
	vec3 l = unit_vector(light_pos - worldpos);
	//��������
	vec3 v = unit_vector(payload.camera->eye - worldpos);
	float n_dot_l = float_max(dot(n, l), 0);
	float n_dot_v = float_max(dot(n, v), 0);

	vec3 color(0.0f, 0.0f, 0.0f);
	
	//���ﲻ�Ǳ����޳�,���Ǽ�����գ����Ҿ��������д�������Ӧ���ǰ��ϻ�����ģ������Ҿ���Ӧ���Ƿ��ߺ͹��ߵĵ���������޸�
	if (n_dot_l>0)
	{
		float roughness = payload.model->roughness(uv);//��������������õ��������괦�Ĵֲڶ�
		float metalness = payload.model->metalness(uv);//��������������õ��������괦�Ľ�����
		float occlusion = payload.model->occlusion(uv);//��������������õ��������괦���ڵ���
		vec3 emission = payload.model->emission(uv);//��������������õ��������괦���Է�����ɫ

		//get albedo
		vec3 albedo = payload.model->diffuse(uv);//�õ��������괦��albode��Ҳ������������ͼ����������ɫֵ
		vec3 temp = vec3(0.04, 0.04, 0.04);//������Ϊ0��Ҳ���Ƿǽ����ķ���ϵ����������Խ�ߣ�����Խ�ߣ����ǽ�����Ϊ0ʱ�ı��׷���ϵ��
		//vec3 temp2 = vec3(1.0f, 1.0f, 1.0f);//Ŀǰû������ɶ����
		vec3 f0 = vec3_lerp(temp, albedo, metalness);

		vec3 F = fresenlschlick_roughness(n_dot_v, f0, roughness);//�����������
		vec3 kD = (vec3(1.0, 1.0, 1.0) - F)*(1 - (double)metalness);//����������ϵ������Ҫ���ˣ�1-�����ȣ�������Ϊ1��Ҳ���Ǵ�����û��������

		//diffuse color�������䲿�֣�������ibl�ģ���������Ӧ����ֱ�Ӳ���Ԥ�˲�������ͼ��
		cubemap_t *irradiance_map = payload.iblmap->irradiance_map;
		vec3 irradiance = cubemap_sampling(n, irradiance_map);//��Ԥ�˲�������ͼ���в���
		
		//�������ʣ�ΪɶҪƽ����
		for (int i = 0; i < 3; i++)
			irradiance[i] = pow(irradiance[i], 2.0f);


		//�����޸ģ��˴���Ӧ��ʽ�������˳��ԦеĲ�����ԭ�򣬾���albedo��Ӧc���������������ɫֵ��kd��������ϵ����irradiance��Li����ʽ��˵Ҫ����PI
		vec3 diffuse = irradiance * kD * albedo/PI;

		//specular color�����淴�䲿�֣�������ibl������˵����Ӧ���Ƕ�lut���в��ң������������������ͷ�������нǵ�����ֵ����������roughness
		vec3 r = unit_vector(2.0*dot(v, n) * n - v);//��view���������ͷ����������˷���������Ҳ���������ķ���
		vec2 lut_uv = vec2(n_dot_v, roughness);//��������˵�ģ�lut��ĺ����������������ͷ������нǵ�����ֵ��Ҳ���ǵ�ˣ��������Ǵֲڶ�
		vec3 lut_sample = texture_sample(lut_uv, payload.iblmap->brdf_lut);//��lut���в������õ�BRDF��Ϣ
		float specular_scale = lut_sample.x();
		float specular_bias = lut_sample.y();
		
		//���淴���irradiance�ļ��㣬
		//��ʵ���ǹ�ʽ��L(x,w0) = F0*LUT.r+LUT.g�������f0��Ȼ�ǻ���������
		vec3 specular = f0 * specular_scale + vec3(specular_bias, specular_bias, specular_bias);

		//�ҵ�Ԥ�˲�������ͼ��mipmap�����ٲ�
		float max_mip_level = (float)(payload.iblmap->mip_levels - 1);

		//����roughness ��λ����Ҫ���Ĳ�mipmap���в���������Ǹ���ʽ��ֱ�Ӽ�ס�Ϳ����ˡ�
		int specular_miplevel = (int)(roughness * max_mip_level + 0.5f);
		
		//�Ըò㼶�Ĺ�����Ϣ���в������õ�Ԥ�˲�������ͼ����ɫֵ
		vec3 prefilter_color = cubemap_sampling(r, payload.iblmap->prefilter_maps[specular_miplevel]);

		for (int i = 0; i < 3; i++)
			prefilter_color[i] = pow(prefilter_color[i], 2.0f);
		
		//����ɾ��Ƕ�Ӧֱ����ˣ���Ԥ�˲���ɫֵ��ԭʼ������ɫֵ������ֱ�Ӷ�Ӧ��ˣ���ʽ�ﱾ��Ҳ����˵ġ�
		specular = cwise_product(prefilter_color, specular);

		//������ɫ����������+���淴��+�Է���
		color = (diffuse + specular) + emission;
	}

	//����ɫ��ӳ�䣬��HDRӳ�䵽LDR����Ϊ�����Ҫ��LDR��Ļ��չʾ
	Reinhard_mapping(color);

	//Ϊʲô����gammaУ���󷴶������ء�����������Ϊ�������ɫ��ӳ������ͽ�����gamma�����ˡ���������
	//color = pow(color, vec3((float)1.0 / 2.2, (float)1.0 / 2.2, (float)1.0 / 2.2));

	return color * 255.f; 
}