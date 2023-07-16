#include "./sample.h"

using namespace std;

//在纹理中，s 轴通常表示水平方向，t 轴表示垂直方向，(0,0) 通常位于纹理的左下角。
/*
计算立方体贴图上给定方向的纹理坐标。
具体来说，采用一个vec3类型的方向向量作为输入，并计算出该方向向量在立方体贴图的哪一个面上，以及该面上的纹理坐标。
函数通过找到给定方向向量中长度最大的分量（即所谓的“主轴”）来确定方向向量所在的立方体面，然后将该面映射到UV空间上的规范化纹理坐标。
最后，该函数返回表示立方体贴图面的索引值。
*/
//立方体贴图的计算通常采用右手系，x轴指向右侧，y轴指向上方，z轴指向屏幕外方向。
static int cal_cubemap_uv(vec3& direction, vec2& uv)
{
	int face_index = -1;//初始时的面坐标设为-1.

	//变量 ma 表示向量 direction 的三个分量中最大的那个（major axis），那么 sc 和 tc 就分别是 z 和 y 分量
	float ma = 0, sc = 0, tc = 0;//通过主轴来判断时哪个面，之后将该面的纹理坐标使用sc，tc的计算求得。
	float abs_x = fabs(direction[0]), abs_y = fabs(direction[1]), abs_z = fabs(direction[2]);

	if (abs_x > abs_y && abs_x > abs_z)			/* major axis -> x */
	{
		ma = abs_x;

		//正x轴方向的面
		if (direction.x() > 0)					/* positive x */
		{
			face_index = 0;
			sc = +direction.z();
			tc = +direction.y();
		}
		else									/* negative x */
		{
			face_index = 1;
			sc = -direction.z();
			tc = +direction.y();
		}
	}
	else if (abs_y > abs_z)						/* major axis -> y */
	{
		ma = abs_y;
		if (direction.y() > 0)					/* positive y */
		{
			face_index = 2;
			sc = +direction.x();	//前两轮都是z变负值
			tc = +direction.z();
		}
		else									/* negative y */
		{
			face_index = 3;
			sc = +direction.x();
			tc = -direction.z();
		}
	}
	else										/* major axis -> z */
	{
		ma = abs_z;
		if (direction.z() > 0)					/* positive z */
		{
			face_index = 4;
			sc = -direction.x();		//最后一轮是x变负值
			tc = +direction.y();
		}
		else									/* negative z */
		{
			face_index = 5;
			sc = +direction.x();
			tc = +direction.y();
		}
	}

	uv[0] = (sc / ma + 1.0f) / 2.0f;//除以ma是为了保证得到的值的范围在[-1,1]内
	uv[1] = (tc / ma + 1.0f) / 2.0f;//除以ma是为了保证得到的值的范围在[-1,1]内

	return face_index;
}

vec3 texture_sample(vec2 uv, TGAImage* image)
{
	//立方体贴图采样里已经通过主轴将值限定到[0,1]里了
	//uv[0] = fmod(uv[0], 1);//fmod，求浮点数除法的余数
	//uv[1] = fmod(uv[1], 1);
	//printf("%f %f\n", uv[0], uv[1]);
	int uv0 = uv[0] * image->get_width();
	int uv1 = uv[1] * image->get_height();
	TGAColor c = image->get(uv0, uv1);
	vec3 ans;
	/*
	通常在计算机图形学中，TGA图像颜色值映射到三维向量时需要颠倒，因为TGA图像的颜色值通常是按照BGR（蓝绿红）的顺序排列的，而不是RGB（红绿蓝）的顺序。
	而在三维图形学中，通常使用的是RGB的颜色表示方式，因此需要将TGA图像中的颜色值颠倒过来。
	在给定的代码中，将TGA颜色值中的RGB通道映射到了vec3中的BGR通道，因此需要进行2-i的颠倒操作，即将B和R通道颠倒过来。
	*/
	for (int i = 0; i < 3; i++)
		ans[2 - i] = (float)c[i] / 255.f;
	return ans;
}

//立方体贴图采样
vec3 cubemap_sampling(vec3 direction, cubemap_t* cubemap)
{
	vec2 uv;
	vec3 color;
	int face_index = cal_cubemap_uv(direction, uv);
	color = texture_sample(uv, cubemap->faces[face_index]);

	//if (fabs(color[0]) < 1e-6&&fabs(color[2]) < 1e-6&&fabs(color[1]) < 1e-6)
	//{

	//	printf("here %d  direction:%f %f %f\n",face_index,direction[0],direction[1],direction[2]);
	//}

	return color;
}

/* for image-based lighting pre-computing */
/*
这个函数是一个经典的Van der Corput 序列生成函数，用于生成一个在 [0,1) 区间内的分布均匀的浮点随机数。
其输入参数 bits 是一个无符号整数，可以看做是随机种子，函数通过对其进行一系列的位操作和变换，将其映射到一个浮点数返回。
具体的操作是将输入值进行位反转（高低位颠倒）、交替位翻转等操作，以得到一个分布更加均匀的随机数序列。
这个函数在计算机图形学、蒙特卡洛渲染等领域都有广泛应用。
*/
//抄的opengl
float radicalInverse_VdC(unsigned int bits) 
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

/*
这个函数用于生成Hammersley采样点，它接收两个参数：i为采样点的索引，N为采样点的总数。
该函数使用Van der Corput序列生成[0,1]区间内的数值，其中一个参数使用i/N，另一个参数使用Van der Corput序列生成的值，最终生成一个二维向量(vec2)，表示二维平面上的一个采样点的坐标。
通过循环调用该函数，可以生成一系列Hammersley采样点，用于渲染图像。
*/
//抄的opengl
//https://blog.csdn.net/lr_shadow/article/details/120446814  hammersley采样点
vec2 hammersley2d(unsigned int i, unsigned int N) //i是采样点的索引，N是总的采样点个数
{
	return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

/*
这个函数生成一个在半球体上均匀采样的向量。其中，参数u和v分别是[0,1]范围内的随机数，用于确定采样点在半球体表面的位置。
函数首先计算半球体表面上的一个点的极坐标角度phi和cosTheta，然后使用这些值计算出一个三维向量，即半球体上的一个采样点。
具体来说，phi表示该点在半球体表面的经度，cosTheta表示该点在半球体表面的纬度。函数返回的vec3向量即为半球体表面上的一个采样点的方向向量。
*/
//返回的向量都是在切线空间中的
vec3 hemisphereSample_uniform(float u, float v) 
{
	float phi = v * 2.0f * PI;
	float cosTheta = 1.0f - u;
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}
//这个函数是用来在半球上根据 cosine-weighted 分布进行采样的
//这个函数生成一个在半球体上均匀采样的向量。其中，参数u和v分别是[0,1]范围内的随机数，用于确定采样点在半球体表面的位置。
//函数首先计算半球体表面上的一个点的极坐标角度phi和cosTheta，然后使用这些值计算出一个三维向量，即半球体上的一个采样点。
//具体来说，phi表示该点在半球体表面的经度，cosTheta表示该点在半球体表面的纬度。函数返回的vec3向量即为半球体表面上的一个采样点的方向向量。
vec3 hemisphereSample_cos(float u, float v) {
	float phi = v * 2.0 * PI;
	float cosTheta = sqrt(1.0 - u);
	float sinTheta = sqrt(1.0 - (double)cosTheta * cosTheta);
	return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

//实现了一个基于GGX的重要性采样算法，用于在PBR中渲染中生成反射/折射光线的方向向量
//该算法的输入参数为一个2D随机数向量Xi、一个表面法向量N和一个粗糙度roughness，输出为一个在以N为法线的半球上采样的随机方向向量。
/*
GGX分布函数是指一种用于计算微平面法线与视线向量之间的几何遮蔽和阴影效果的函数，是PBR渲染中常用的分布函数之一。
而重要性采样是指在采样过程中使用概率密度函数（PDF）来引导采样，从而提高采样效率和采样质量的方法。
*/
//抄的opengl，而且，这些和他妈的PBR里面的重了
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	//计算粗糙度对应的Alpha值：GGX分布函数的参数alpha等于roughness的平方。
	float a = roughness * roughness;

	//在半球空间内进行采样：用随机数向量的第一维u计算phi角度，用u对应的伪随机数值计算cosTheta，再计算sinTheta。
	float phi = 2.0 * PI * Xi.x();
	float cosTheta = sqrt((1.0 - Xi.y()) / (1.0 + ((double)a*a - 1.0) * Xi.y()));
	float sinTheta = sqrt(1.0 - (double)cosTheta * cosTheta);

	//从半球空间内的采样点H，构建出采样方向向量sampleVec：先从表面法向量N推导出tangent和bitangent向量，然后用H向量的分量乘以tangent，bitangent和N的线性组合得到采样方向向量sampleVec。
	//from spherical coordinates to cartesian coordinates
	vec3 H;
	H[0] = cos(phi) * sinTheta;
	H[1] = sin(phi) * sinTheta;
	H[2] = cosTheta;

	//为了下面将从切线空间转换到世界空间做准备
	vec3 up = abs(N.z()) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = unit_vector(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	// 返回归一化的采样方向向量sampleVec。
	// from tangent-space vector to world-space sample vector
	vec3 sampleVec = tangent * H.x() + bitangent * H.y() + N * H.z();
	return unit_vector(sampleVec);
}
/*
这个函数计算了GGX microfacet模型中的几何遮蔽因子。几何遮蔽因子是用来描述微观表面结构对于光的反射和折射的影响的，它是一个介于0和1之间的值，表示在一个表面微元上随机选取两个观察方向时，能否看到光源。
在这个函数中，输入参数n_dot_v是视角方向向量V和法线向量N的点积，即cos(θ)。roughness是表面的粗糙度，越大表示表面越粗糙。
函数首先计算了一个系数k，该系数与表面的粗糙度相关，用来控制函数的衰减。然后根据GGX微平面模型中的公式，计算了几何遮蔽因子，即G_SchlickGGX(n_dot_v, k) = n_dot_v / (n_dot_v * (1 - k) + k)
其中，n_dot_v表示视角方向向量V和法线向量N的点积，k表示衰减系数，根据这个公式，当k=0时，遮蔽因子为1，表示完全可见，即表面平整。
当k越大时，遮蔽因子越小，表示表面越粗糙，微观结构对光的反射和折射影响越大，这是因为微平面在不同角度下对于光的反射和折射有不同的效果。
*/
//抄的opengl，而且，这些和PBR里面的重了，就不能不加static，起到复用吗
static float SchlickGGX_geometry(float n_dot_v, float roughness)
{
	float r = (1 + roughness);
	float k = (double)r * r / 8.0;
	//k = roughness * roughness / 2.0f; //learnopengl里面没有这一行
	return n_dot_v / (n_dot_v*(1 - k) + k);
}
/*
这个函数是用来计算基于Schlick GGX分布的法线分布函数(Normal Distribution Function, NDF)和几何函数(geometry function)来计算Smith几何遮蔽函数(Geometry-SchlickSmith Shadowing Function)的结果。
在渲染过程中，为了计算遮蔽阴影和环境光照，需要计算每个表面点在表面法线方向和光线方向下的遮蔽率，即遮蔽函数。
其中，Smith几何遮蔽函数是比较常用的一种计算遮蔽的函数，其计算方式涉及到表面法线分布函数和几何函数。
在这个函数中，首先使用Schlick GGX分布函数计算出入射和视线在表面法线方向上的几何遮蔽，分别存储在变量g1和g2中。然后将g1和g2相乘得到最终的Smith几何遮蔽函数的结果。
*/
//抄的opengl，而且，这些和PBR里面的重了，就不能不加static，起到复用吗
static float geometry_Smith(float n_dot_v, float n_dot_l, float roughness)
{
	float g1 = SchlickGGX_geometry(n_dot_v, roughness);
	float g2 = SchlickGGX_geometry(n_dot_l, roughness);

	return g1 * g2;
}

/*
这段代码的作用是设置在立方体贴图上某个像素的法向量的坐标值。
立方体贴图是由6个面组成的立方体，每个面都对应了不同的方向（正/负 x/y/z），而每个像素可以看作是立方体贴图上的一个小立方体，
因此需要通过参数face_id来指定该像素所在的面，然后根据该像素在该面上的坐标(x,y)，计算出该像素的法向量在世界空间中的坐标值(x_coord,y_coord,z_coord)。
具体的实现方式是根据面的方向和像素坐标计算出在该面上的坐标，再通过简单的数学运算转换为世界空间中的坐标。
*/
/* set normal vector for different face of cubemap */
void set_normal_coord(int face_id, int x, int y, float &x_coord, float &y_coord, float &z_coord, float length = 255)
{
	switch (face_id)
	{
	case 0:   //positive x (right face)
		x_coord = 0.5f;
		y_coord = -0.5f + y / length;
		z_coord = -0.5f + x / length;
		break;
	case 1:   //negative x (left face)		
		x_coord = -0.5f;
		y_coord = -0.5f + y / length;
		z_coord = 0.5f - x / length;
		break;
	case 2:   //positive y (top face)
		x_coord = -0.5f + x / length;
		y_coord = 0.5f;
		z_coord = -0.5f + y / length;
		break;
	case 3:   //negative y (bottom face)
		x_coord = -0.5f + x / length;
		y_coord = -0.5f;
		z_coord = 0.5f - y / length;
		break;
	case 4:   //positive z (back face)
		x_coord = 0.5f - x / length;
		y_coord = -0.5f + y / length;
		z_coord = 0.5f;
		break;
	case 5:   //negative z (front face)
		x_coord = -0.5f + x / length;
		y_coord = -0.5f + y / length;
		z_coord = -0.5f;
		break;
	default:
		break;
	}
}


/* specular part */
void generate_prefilter_map(int thread_id, int face_id, int mip_level,TGAImage &image)
{
	int factor = 1;
	for (int temp = 0; temp < mip_level; temp++)
		factor *= 2;
	int width = 512 / factor;
	int height = 512 / factor;


	if (width < 64)
		width = 64;


	const char* modelname5[] =
	{
		"obj/gun/Cerberus.obj",
		"obj/skybox4/box.obj",
	};

	float roughness[10];
	for (int i = 0; i < 10; i++)
		roughness[i] = i * (1.0 / 9.0);
	roughness[0] = 0; roughness[9] = 1;

	/* for multi-thread */
	//int interval = width / thread_num;
	//int start = thread_id * interval;
	//int end = (thread_id + 1) * interval;
	//if (thread_id == thread_num - 1)
	//	end = width;

	Model* model[1];
	model[0] = new Model(modelname5[1], true);

	payload_t p;
	p.model = model[0];

	vec3 prefilter_color(0, 0, 0);

	int x, y;
	for (x = 0; x < height; x++)
	{
		for (y = 0; y < width; y++)
		{
			float x_coord, y_coord, z_coord;
			set_normal_coord(face_id, x, y, x_coord, y_coord, z_coord, float(width - 1));

			vec3 normal = vec3(x_coord, y_coord, z_coord);
			normal = unit_vector(normal);					//z-axis
			vec3 up = fabs(normal[1]) < 0.999f ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f);
			vec3 right = unit_vector(cross(up, normal));	//x-axis
			up = cross(normal, right);						//y-axis

			vec3 r = normal;
			vec3 v = r;

			prefilter_color = vec3(0, 0, 0);
			//重要性采样
			float total_weight = 0.0f;
			int numSamples = 1024;
			for (int i = 0; i < numSamples; i++)
			{
				vec2 Xi = hammersley2d(i, numSamples);
				vec3 h = ImportanceSampleGGX(Xi, normal, roughness[mip_level]);
				vec3 l = unit_vector(2.0*dot(v, h) * h - v);

				vec3 radiance = cubemap_sampling(l, p.model->environment_map);
				float n_dot_l = float_max(dot(normal, l), 0.0);

				if (n_dot_l > 0)
				{
					prefilter_color += radiance * n_dot_l;
					total_weight += n_dot_l;
				}
			}

			prefilter_color = prefilter_color / total_weight;
			//cout << irradiance << endl;
			int red	  = float_min(prefilter_color.x() * 255.0f, 255);
			int green = float_min(prefilter_color.y() * 255.0f, 255);
			int blue  = float_min(prefilter_color.z() * 255.0f, 255);

			//cout << irradiance << endl;
			TGAColor temp(red, green, blue);
			image.set(x, y, temp);
		}
		cout << x / 512.f << endl;
	}
}

/* diffuse part */
void generate_irradiance_map(int thread_id, int face_id,TGAImage &image)
{
	int x, y;
	const char* modelname5[] =
	{
		"obj/gun/Cerberus.obj",
		"obj/skybox4/box.obj",
	};

	/* for multi-thread */
	//int interval = 256 / thread_num;
	//int start = thread_id * interval;
	//int end = (thread_id + 1) * interval;
	//if (thread_id == thread_num - 1)
	//	end = 256;

	Model *model[1];
	model[0] = new Model(modelname5[1], 1);

	payload_t p;
	p.model = model[0];

	vec3 irradiance(0, 0, 0);
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 256; y++)
		{
			float x_coord, y_coord, z_coord;
			set_normal_coord(face_id, x, y, x_coord, y_coord, z_coord);
			vec3 normal = unit_vector(vec3(x_coord, y_coord, z_coord));					 //z-axis
			vec3 up = fabs(normal[1]) < 0.999f ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f);
			vec3 right = unit_vector(cross(up, normal));								 //tagent x-axis
			up = cross(normal, right);					                                 //tagent y-axis

			irradiance = vec3(0, 0, 0);
			float sampleDelta = 0.025f;
			int numSamples = 0;
			for (float phi = 0.0f; phi < 2.0 * PI; phi += sampleDelta)
			{
				for (float theta = 0.0f; theta < 0.5 * PI; theta += sampleDelta)
				{
					// spherical to cartesian (in tangent space)
					vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
					// tangent space to world
					vec3 sampleVec = tangentSample.x() * right + tangentSample.y() * up + tangentSample.z() * normal;
					sampleVec = unit_vector(sampleVec);
					vec3 color = cubemap_sampling(sampleVec, p.model->environment_map);

					irradiance += color * sin(theta) *cos(theta);
					numSamples++;
				}
			}

			irradiance = PI * irradiance * (1.0f / numSamples);
			int red = float_min(irradiance.x() * 255.0f, 255);
			int green = float_min(irradiance.y() * 255.0f, 255);
			int blue = float_min(irradiance.z() * 255.0f, 255);

			TGAColor temp(red, green, blue);
			image.set(x, y, temp);
		}
		printf("%f% \n", x / 256.0f);
	}
}

/* lut part */
vec3 IntegrateBRDF(float NdotV, float roughness)
{
	// 由于各向同性，随意取一个 V 即可
	vec3 V;
	V[0] = 0;
	V[1] = sqrt(1.0 - (double)NdotV * NdotV);
	V[2] = NdotV;

	float A = 0.0f;
	float B = 0.0f;
	float C = 0.0f;

	vec3 N = vec3(0.0, 0.0, 1.0);

	const int SAMPLE_COUNT = 1024;
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		// generates a sample vector that's biased towards the
		// preferred alignment direction (importance sampling).
		vec2 Xi = hammersley2d(i, SAMPLE_COUNT);

		{ // A and B
			vec3 H = ImportanceSampleGGX(Xi, N, roughness);
			vec3 L = unit_vector(2.0 * dot(V, H) * H - V);

			float NdotL = float_max(L.z(), 0.0);
			float NdotV = float_max(V.z(), 0.0);
			float NdotH = float_max(H.z(), 0.0);
			float VdotH = float_max(dot(V, H), 0.0);

			if (NdotL > 0.0)
			{
				float G = geometry_Smith(NdotV, NdotL, roughness);
				float G_Vis = (G * VdotH) / (NdotH * NdotV);
				float Fc = pow(1.0 - VdotH, 5.0);

				A += (1.0 - Fc) * G_Vis;
				B += Fc * G_Vis;
			}
		}
	}
	return vec3(A, B, C) / float(SAMPLE_COUNT);
}

/* traverse all 2d coord for lut part */
void calculate_BRDF_LUT(TGAImage& image)
{
	int i, j;
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 256; j++)
		{
			vec3 color;

			if (i == 0)
				color = IntegrateBRDF(0.002f, j / 256.0f);
			else
				color = IntegrateBRDF(i / 256.0f, j / 256.0f);
			
			//cout << irradiance << endl;
			
			int red   = float_min(color.x() * 255.0f, 255);
			int green = float_min(color.y() * 255.0f, 255);
			int blue  = float_min(color.z() * 255.0f, 255);

			//cout << irradiance << endl;
			TGAColor temp(red, green, blue);
			image.set(i, j, temp);
		}
	}
}

/* traverse all mipmap level for prefilter map */
void foreach_prefilter_miplevel(TGAImage &image)
{
	const char *faces[6] = { "px", "nx", "py", "ny", "pz", "nz" };
	char paths[6][256];
	const int thread_num = 4;

	for (int mip_level = 8; mip_level <10; mip_level++)
	{
		for (int j = 0; j < 6; j++) {
			sprintf_s(paths[j], "%s/m%d_%s.tga", "./obj/common2", mip_level,faces[j]);
		}
		int factor = 1;
		for (int temp = 0; temp < mip_level; temp++)
			factor *= 2;
		int w = 512 / factor;
		int h = 512 / factor;

		if (w < 64)
			w = 64;
		if (h < 64)
			h = 64;
		cout << w << h << endl;

		image = TGAImage(w, h, TGAImage::RGB);
		for (int face_id = 0; face_id < 6; face_id++)
		{
			std::thread thread[thread_num];
			for (int i = 0; i < thread_num; i++)
				thread[i] = std::thread(generate_prefilter_map, i, face_id,mip_level,std::ref(image));
			for (int i = 0; i < thread_num; i++)
				thread[i].join();

			//calculate_BRDF_LUT();
			image.flip_vertically(); // to place the origin in the bottom left corner of the image
			image.write_tga_file(paths[face_id]);
		}
	}
}

/* traverse all faces of cubemap for irradiance map */
void foreach_irradiance_map(TGAImage& image)
{
	const char *faces[6] = { "px", "nx", "py", "ny", "pz", "nz" };
	char paths[6][256];
	const int thread_num = 4;

	for (int j = 0; j < 6; j++) 
	{
		sprintf_s(paths[j], "%s/i_%s.tga", "./obj/common2", faces[j]);
	}
	image = TGAImage(256, 256, TGAImage::RGB);
	for (int face_id = 0; face_id < 6; face_id++)
	{
		std::thread thread[thread_num];
		for (int i = 0; i < thread_num; i++)
			thread[i] = std::thread(generate_irradiance_map, i, face_id, std::ref(image));
		for (int i = 0; i < thread_num; i++)
			thread[i].join();

		image.flip_vertically(); // to place the origin in the bottom left corner of the image
		image.write_tga_file(paths[face_id]);
	}
}