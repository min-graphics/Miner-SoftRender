#include "./shader.h"
#include "../core/tgaimage.h"
#include "../core/sample.h"
#include <cmath>


/*
D项:法线分布函数
微平面理论中的ggx分布函数。
在该分布函数中，一个微平面的法向量与视角向量之间的夹角和法向量与光源向量之间的夹角共同决定了微平面反射的亮度。
实现金属或塑料等表面的微观粗糙度，以及提高实时渲染引擎中的反射表现效果。
*/
static float GGX_distribution(float n_dot_h, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;

	float n_dot_h_2 = n_dot_h * n_dot_h;
	float factor = n_dot_h_2 * (alpha2 - 1) + 1;
	return alpha2 / (PI * factor * factor);
}


//F:菲涅尔项是因观察角度与反射平面方向的夹角而引起的反射程度不同
//所以菲尼尔项计算的是微平面法向(真正有贡献的微平面)与观察方向的夹角
//F:菲涅尔项：主要是用来表示观察角度和反射之间的关系，也就是说不通观察角度下，反射光所占比例的多少。
//真正精确的菲尼尔项是要考虑光线的S极化，P极化，之后将两个值做平均，但是这里我们简单的使用施利克近似。
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


//G:几何遮蔽项，主要表示微表面的细小平面的自遮挡情况。一般用史密斯方程来表示，分别计算入射光线被遮挡的情况和视线被遮挡的情况，但是要注意这里的k根据是直接光照还是IBL间接光照，被α进行重定向。
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
//G:史密斯方程，两个遮挡比相乘，几何遮挡计算的是物体宏观表面的影响，所以是宏观法向
static float geometry_Smith(float n_dot_v, float n_dot_l, float roughness)
{
	float g1 = SchlickGGX_geometry(n_dot_v, roughness);
	float g2 = SchlickGGX_geometry(n_dot_l, roughness);

	return g1 * g2;
}


/*
这个函数也是用来进行色域映射的。。。把hdr转到ldr,这个函数其实就是Reinhard mapping的一部分。

这个函数实现了一种名为 "ACES" (Academy Color Encoding System) 的色彩管理系统中的色彩映射算法。
该算法通过将输入值 (通常是色彩值) 映射到新的输出值来改变颜色的外观。

在函数中，给定一个浮点数值 value，该函数通过将该值代入预定义的一些常量 (a、b、c、d、e)，并进行一些简单的数学计算来计算映射后的值。
最终结果通过调用 float_clamp 函数来确保它在 0 到 1 之间。
这种色彩映射技术通常用于将一个色彩空间中的颜色值映射到另一个色彩空间中的颜色值，或者用于进行颜色校正和调整，以实现更好的图像质量或更好的视觉效果。
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


//Reinhard色调映射方法，输入的vec3是HDR，输出的color是LDR颜色。目的是使HDR图像在LDR显示设备上更好的呈现出来。
//在LDR中，色彩被限制在了0-1之间被表示，这样会导致失去很多的细节，比如灯泡和太阳，可能都会是一片白。太亮的地方和太暗的地方细节丢失。HDR就是通过色调映射，将LDR的转入到高动态范围中，
//能保留更多的显示细节。
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


//计算法线贴图纹理坐标的法线向量
//接受的参数分别是，原始法线向量，世界坐标数组，纹理坐标数组，当前片段的纹理坐标以及法线贴图的TGA图像
/*
法线贴图中的法线向量定义在切线空间中，在切线空间中，法线永远指向正Z方向。
法线相对于单个三角形的本地参考框架。
它就像法线贴图向量的本地空间；它们都被定义为指向正z方向，无论最终变换到什么方向。
使用一个特定的矩阵我们就能将本地/切线空间中的法线向量转成世界或视图空间下，使它们转向到最终的贴图表面的方向。
之后我们把切线空间的Z方向和表面的法线方向对齐即可。
这种矩阵叫做TBN矩阵这三个字母分别代表tangent、bitangent和normal向量。这是建构这个矩阵所需的向量。
要建构这样一个把切线空间转变为不同空间的变异矩阵，我们需要三个相互垂直的向量，它们沿一个表面的法线贴图对齐于：上、右、前；这和我们在摄像机教程中做的类似。

计算出切线和副切线并不像法线向量那么容易。从图中可以看到法线贴图的切线和副切线与纹理坐标的两个方向对齐。
我们就是用到这个特性计算每个表面的切线和副切线的。需要用到一些数学才能得到它们；
*/
static vec3 cal_normal(vec3& normal, vec3* world_coords, const vec2* uvs, const vec2& uv, TGAImage* normal_map)
{
	//learnOpenGL里讲解过，这里是采样法线贴图里的纹理，并将其还原成法线值。
	//从法线贴图中采样得到一个法线样本
	vec3 sample = texture_sample(uv, normal_map);
	//modify the range 0 ~ 1 to -1 ~ +1
	//对采样得到的法线样本进行范围映射，将其从0到1的分为转换为-1到+1的范围。
	sample = vec3(sample[0] * 2 - 1, sample[1] * 2 - 1, sample[2] * 2 - 1);

	//下面就要计算TBN矩阵

	//calculate the difference in UV coordinate
	//计算了纹理坐标的差异，计算了两个边的插值，x1，y1和x2，y2.并用这些插值计算了一个行列式值(det)。
	float x1 = uvs[1][0] - uvs[0][0];
	float y1 = uvs[1][1] - uvs[0][1];
	float x2 = uvs[2][0] - uvs[0][0];
	float y2 = uvs[2][1] - uvs[0][1];

	float det = (x1 * y2 - x2 * y1);

	//calculate the difference in world pos
	//计算了世界坐标之间的差异，通过计算两个边之间的插值(e1和e2)
	vec3 e1 = world_coords[1] - world_coords[0];
	vec3 e2 = world_coords[2] - world_coords[0];

	//calculate tangent-axis and bitangent-axis
	//计算了切线轴tangent-axis和副切线轴bitangent-axis
	vec3 t = e1 * y2 + e2 * (-y1);
	vec3 b = e1 * (-x2) + e2 * x1;
	t /= det;//通过除以之前计算的行列式值来归一化
	b /= det;

	//施密特正交化，保证3个向量都是正交的，移除与目前轴相关的分量。
	//Schmidt orthogonalization
	normal = unit_vector(normal);
	t = unit_vector(t - dot(t, normal)*normal);
	b = unit_vector(b - dot(b, normal)*normal - dot(b, t)*t);

	//使用切线轴、副切线轴和原始切线向量按权重组合法线样本，得到最终的新法线向量。将新法线向量作为函数的返回值。
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

	//only model matrix can change normal vector//只有模型矩阵可以改变法线向量
	for (int i = 0; i < 3; i++)
	{
		payload.worldcoord_attri[nvertex][i]     = temp_vert[i];
		payload.in_worldcoord[nvertex][i]        = temp_vert[i];
		payload.normal_attri[nvertex][i]         = temp_normal[i];
		payload.in_normal[nvertex][i]            = temp_normal[i];
	}
}

//添加进去了
vec3 PBRShader::direct_fragment_shader(float alpha, float beta, float gamma)
{
	vec3 CookTorrance_brdf;//用来存储cooktorrance brdf的结果

	vec3 light_pos = vec3(2, 1.5, 5);//光源的位置
	vec3 radiance = vec3(3,3,3);//光源的辐照度，也就是光源的颜色

	//for reading easily
	vec4* clip_coords = payload.clipcoord_attri;//裁剪空间的坐标
	vec3* world_coords = payload.worldcoord_attri;//世界坐标
	vec3* normals = payload.normal_attri;//法线向量
	vec2* uvs = payload.uv_attri;//uv纹理坐标

	//interpolate attribute,对属性进行插值
	float Z = 1.0 / ((double)alpha / clip_coords[0].w() + beta / clip_coords[1].w() + gamma / clip_coords[2].w());
	vec3 normal = (alpha * normals[0] / clip_coords[0].w() + beta * normals[1] / clip_coords[1].w() +
		gamma * normals[2] / clip_coords[2].w()) * Z;
	vec2 uv = (alpha*uvs[0] / clip_coords[0].w() + beta * uvs[1] / clip_coords[1].w() +
		gamma * uvs[2] / clip_coords[2].w()) * Z;
	vec3 worldpos = (alpha*world_coords[0] / clip_coords[0].w() + beta * world_coords[1] / clip_coords[1].w() +
		gamma * world_coords[2] / clip_coords[2].w()) * Z;

	vec3 l = unit_vector(light_pos - worldpos);//光源向量
	vec3 n = unit_vector(normal);//法向量
	vec3 v = unit_vector(payload.camera->eye - worldpos);//视线向量
	vec3 h = unit_vector(l + v);//半程向量

	//最小值是0，应该在半球上积分，这里应该是积分域
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
//这里三个参数，α，β，γ是用来进行插值的
vec3 PBRShader::fragment_shader(float alpha, float beta, float gamma)
{
	vec3 CookTorrance_brdf;//用于存储cook-torrance brdf的结果

	vec3 light_pos = vec3(2, 1.5, 5);//表示光源的位置
	vec3 radiance = vec3(3, 3, 3);//表示光源的辐射度，也就是光源的颜色

	//for reading easily，主要是那些顶点属性，也就是顶点着色器传过来的，声明了一些指向顶点属性的指针。
	vec4* clip_coords = payload.clipcoord_attri;//指向裁剪空间坐标
	vec3* world_coords = payload.worldcoord_attri;//指向世界坐标
	vec3* normals = payload.normal_attri;//指向法线坐标
	vec2* uvs = payload.uv_attri;//指向纹理坐标

	//interpolate attribute
	float Z = 1.0 / (alpha / (double)clip_coords[0].w() + beta / clip_coords[1].w() + gamma / clip_coords[2].w());//使用重心坐标对裁剪空间的深度值进行加权平均，得到片段的深度值。
	
	//对法线向量，纹理坐标，世界坐标进行插值
	vec3 normal = (alpha * normals[0] / clip_coords[0].w() + beta * normals[1] / clip_coords[1].w() +
		gamma * normals[2] / clip_coords[2].w()) * Z;
	vec2 uv = (alpha * uvs[0] / clip_coords[0].w() + beta * uvs[1] / clip_coords[1].w() +
		gamma * uvs[2] / clip_coords[2].w()) * Z;
	vec3 worldpos = (alpha * world_coords[0] / clip_coords[0].w() + beta * world_coords[1] / clip_coords[1].w() +
		gamma * world_coords[2] / clip_coords[2].w()) * Z;

	//从法线贴图中采样得到法线向量
	if (payload.model->normalmap)
	{
		normal = cal_normal(normal, world_coords, uvs, uv, payload.model->normalmap);
	}

	//法线向量
	vec3 n = unit_vector(normal);
	vec3 l = unit_vector(light_pos - worldpos);
	//视线向量
	vec3 v = unit_vector(payload.camera->eye - worldpos);
	float n_dot_l = float_max(dot(n, l), 0);
	float n_dot_v = float_max(dot(n, v), 0);

	vec3 color(0.0f, 0.0f, 0.0f);
	
	//这里不是背部剔除,而是计算光照，但我觉得这里有错误，这里应该是按断积分域的，所以我觉得应该是法线和光线的点积。自行修改
	if (n_dot_l>0)
	{
		float roughness = payload.model->roughness(uv);//进行纹理采样，得到纹理坐标处的粗糙度
		float metalness = payload.model->metalness(uv);//进行纹理采样，得到纹理坐标处的金属度
		float occlusion = payload.model->occlusion(uv);//进行纹理采样，得到纹理坐标处的遮挡度
		vec3 emission = payload.model->emission(uv);//进行纹理采样，得到纹理坐标处的自发光颜色

		//get albedo
		vec3 albedo = payload.model->diffuse(uv);//得到纹理坐标处的albode，也就是漫反射贴图的漫反射颜色值
		vec3 temp = vec3(0.04, 0.04, 0.04);//金属度为0，也就是非金属的反射系数，金属度越高，反射越高，这是金属度为0时的保底反射系数
		//vec3 temp2 = vec3(1.0f, 1.0f, 1.0f);//目前没看出有啥用来
		vec3 f0 = vec3_lerp(temp, albedo, metalness);

		vec3 F = fresenlschlick_roughness(n_dot_v, f0, roughness);//计算菲涅尔项
		vec3 kD = (vec3(1.0, 1.0, 1.0) - F)*(1 - (double)metalness);//计算漫反射系数，不要忘了（1-金属度）金属度为1，也就是纯金属没有漫反射

		//diffuse color，漫反射部分，由于是ibl的，所以这里应该是直接采样预滤波环境贴图了
		cubemap_t *irradiance_map = payload.iblmap->irradiance_map;
		vec3 irradiance = cubemap_sampling(n, irradiance_map);//对预滤波环境贴图进行采样
		
		//充满疑问，为啥要平方？
		for (int i = 0; i < 3; i++)
			irradiance[i] = pow(irradiance[i], 2.0f);


		//自我修改，此处对应公式，进行了除以π的操作，原因，觉得albedo对应c，都是漫反射的颜色值，kd是漫反射系数，irradiance是Li，公式来说要除以PI
		vec3 diffuse = irradiance * kD * albedo/PI;

		//specular color，镜面反射部分，由于是ibl，所以说这里应该是对lut进行查找，横坐标是视线向量和法向量间夹角的余弦值，纵坐标是roughness
		vec3 r = unit_vector(2.0*dot(v, n) * n - v);//由view方向向量和法向量计算了反射向量，也就是入射光的方向
		vec2 lut_uv = vec2(n_dot_v, roughness);//正如我所说的，lut表的横坐标是视线向量和法向量夹角的余弦值，也就是点乘，纵坐标是粗糙度
		vec3 lut_sample = texture_sample(lut_uv, payload.iblmap->brdf_lut);//对lut进行采样，得到BRDF信息
		float specular_scale = lut_sample.x();
		float specular_bias = lut_sample.y();
		
		//镜面反射的irradiance的计算，
		//其实就是公式，L(x,w0) = F0*LUT.r+LUT.g，这里的f0依然是基础反射率
		vec3 specular = f0 * specular_scale + vec3(specular_bias, specular_bias, specular_bias);

		//找到预滤波环境贴图的mipmap最多多少层
		float max_mip_level = (float)(payload.iblmap->mip_levels - 1);

		//根据roughness 定位到底要在哪层mipmap进行采样，这就是个公式，直接记住就可以了。
		int specular_miplevel = (int)(roughness * max_mip_level + 0.5f);
		
		//对该层级的光照信息进行采样，得到预滤波环境贴图的颜色值
		vec3 prefilter_color = cubemap_sampling(r, payload.iblmap->prefilter_maps[specular_miplevel]);

		for (int i = 0; i < 3; i++)
			prefilter_color[i] = pow(prefilter_color[i], 2.0f);
		
		//这个成就是对应直接相乘，对预滤波颜色值和原始镜面颜色值进行了直接对应相乘，公式里本来也是相乘的。
		specular = cwise_product(prefilter_color, specular);

		//最终颜色等于漫反射+镜面反射+自发光
		color = (diffuse + specular) + emission;
	}

	//进行色域映射，将HDR映射到LDR，因为最后是要在LDR屏幕上展示
	Reinhard_mapping(color);

	//为什么加上gamma校正后反而不对呢。。。。？因为他妈的在色域映射里面就进行了gamma矫正了。。。无语
	//color = pow(color, vec3((float)1.0 / 2.2, (float)1.0 / 2.2, (float)1.0 / 2.2));

	return color * 255.f; 
}