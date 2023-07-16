#include "./scene.h"

//从文件中加载纹理
TGAImage* texture_from_file(const char* file_name)
{
	TGAImage* texture = new TGAImage();
	texture->read_tga_file(file_name);
	//texture->flip_vertically();
	return texture;
}

//生成cubemap，环境贴图
cubemap_t* cubemap_from_files(const char* positive_x, const char* negative_x,
	const char* positive_y, const char* negative_y,
	const char* positive_z, const char* negative_z)
{
	cubemap_t* cubemap = new cubemap_t();

	cubemap->faces[0] = texture_from_file(positive_x);
	cubemap->faces[1] = texture_from_file(negative_x);
	cubemap->faces[2] = texture_from_file(positive_y);
	cubemap->faces[3] = texture_from_file(negative_y);
	cubemap->faces[4] = texture_from_file(positive_z);
	cubemap->faces[5] = texture_from_file(negative_z);
	return cubemap;
}

//这个函数主要是从IBL的6个环境贴图中提取数据，填补满payload结构体中的iblmap结构体,只有PBR中才使用到了这个函数，因为只有PBR中才使用到了IBL
void load_ibl_map(payload_t& p, const char* env_path)
{
	iblmap_t* iblmap = new iblmap_t();//主要是填满iblmap

	const char* faces[6] = { "px", "nx", "py", "ny", "pz", "nz" };

	char paths[6][256];//路径数组，分成6个面，编号0-5，之后

	iblmap->mip_levels = 10;//0-9，共10个，这是给mipmap，主要是IBL的specular当中使用到了mipmap，并需要通过roughness通过公式来选取合适的层级，来求取光照信息。

	/* diffuse environment map *///这是i_nx，i_ny，i_nz，i_px，i_py，i_pz那6张图，用来加载diffuse的图的
	for (int j = 0; j < 6; j++)
	{
		sprintf_s(paths[j], "%s/i_%s.tga", env_path, faces[j]);
	}
	iblmap->irradiance_map = cubemap_from_files(paths[0], paths[1], paths[2],paths[3], paths[4], paths[5]);

	/* specular environment maps *///这个主要是根据roughness进行求取光照
	for (int i = 0; i < iblmap->mip_levels; i++)
	{
		for (int j = 0; j < 6; j++) 
		{
			//sprintf_s将数据格式化的输出到字符串中，load_ibl_map(shader_pbr->payload, "../obj/common2");
			sprintf_s(paths[j], "%s/m%d_%s.tga", env_path, i, faces[j]);
		}
		iblmap->prefilter_maps[i] = cubemap_from_files(paths[0], paths[1],paths[2], paths[3], paths[4], paths[5]);
	}
	
	/* brdf lookup texture */
	iblmap->brdf_lut = texture_from_file("../obj/common/BRDF_LUT.tga");

	p.iblmap = iblmap;
}

//参数解释，这里的model是模型数组，m是模型个数，shader数组是
void build_qiyana_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
	m = 3;
	const char* modelname[] =
	{
		"../obj/qiyana/qiyanabody.obj",
		"../obj/qiyana/qiyanahair.obj",
		"../obj/qiyana/qiyanaface.obj",
	};
	camera->eye = vec3(0, 1, 2);
	PhongShader* shader_phong = new PhongShader();//使用phongshader进行渲染

	int vertex = 0, face = 0;
	const char* scene_name = "qiyana";
	
	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], false, true);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	//给shader设置相机和投影矩阵
	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}
//参数解释，这里的model是模型数组，m是模型个数，shader数组是
void build_yongzhuangfuhua_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
	m = 3;
	const char* modelname[] =
	{
		"../obj/yongzhuangfuhua/yongzhuangfuhuabody.obj",
		"../obj/yongzhuangfuhua/yongzhuangfuhuahair.obj",
		"../obj/yongzhuangfuhua/yongzhuangfuhuaface.obj",
	};
	camera->eye = vec3(0, 1, 2);
	PhongShader* shader_phong = new PhongShader();//使用phongshader进行渲染

	int vertex = 0, face = 0;
	const char* scene_name = "yongzhuangfuhua";

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], false, true);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	//给shader设置相机和投影矩阵
	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}

void build_sanyueqi_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
	m = 5;
	const char* modelname[] =
	{
		"../obj/sanyueqi/sanyueqibody.obj",
		"../obj/sanyueqi/sanyueqihair.obj",
		"../obj/sanyueqi/sanyueqiface.obj",
		"../obj/sanyueqi/sanyueqishangyi.obj",
		"../obj/sanyueqi/sanyueqiweapon.obj"
	};
	camera->eye = vec3(0, 1, 2);
	PhongShader* shader_phong = new PhongShader();//使用phongshader进行渲染

	int vertex = 0, face = 0;
	const char* scene_name = "sanyueqi";

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], false, true);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	//给shader设置相机和投影矩阵
	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);

}

void build_yinlang_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
	m = 5;
	const char* modelname[] =
	{
		"../obj/yinlang/yinlangbody.obj",
		"../obj/yinlang/yinlanghair.obj",
		"../obj/yinlang/yinlangface.obj",
		"../obj/yinlang/yinlangshangyi.obj",
		"../obj/yinlang/yinlangweapon.obj"
	};
	camera->eye = vec3(0, 1, 2);
	PhongShader* shader_phong = new PhongShader();//使用phongshader进行渲染

	int vertex = 0, face = 0;
	const char* scene_name = "yinlang";

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], false, true);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	//给shader设置相机和投影矩阵
	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);

}

void build_mona_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
	m = 3;
	const char* modelname[] =
	{
		"../obj/mona/monabody.obj",
		"../obj/mona/monahair.obj",
		"../obj/mona/monaface.obj"
	};
	camera->eye = vec3(0, 1, 2);
	PhongShader* shader_phong = new PhongShader();//使用phongshader进行渲染

	int vertex = 0, face = 0;
	const char* scene_name = "mona";

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], false, true);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	//给shader设置相机和投影矩阵
	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}

void build_difa_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
}

void build_helmet_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
	m = 2;
	const char* modelname[] =
	{
		"../obj/helmet/helmet.obj",
		"../obj/skybox4/box.obj",
	};
	camera->eye = vec3(0, 1, 5);
	PBRShader* shader_pbr = new PBRShader();
	SkyboxShader* shader_sky = new SkyboxShader();

	int vertex = 0, face = 0;
	const char* scene_name = "helmet";

	//一共只有两个模型，所以说就没有必要写循环了
	model[0] = new Model(modelname[0], 0, 0); vertex += model[0]->nverts(); face += model[0]->nfaces();
	model[1] = new Model(modelname[1], 1, 0); vertex += model[1]->nverts(); face += model[1]->nfaces();

	shader_pbr->payload.camera_perp_matrix = perspective;
	shader_pbr->payload.camera = camera;
	shader_sky->payload.camera_perp_matrix = perspective;
	shader_sky->payload.camera = camera;

	load_ibl_map(shader_pbr->payload, "../obj/common2");

	*shader_use = shader_pbr;
	*shader_skybox = shader_sky;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}

void build_fuhua_scene(Model **model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera)
{
	m = 4;
	const char* modelname[] = 
	{
		"../obj/fuhua/fuhuabody.obj",
		"../obj/fuhua/fuhuahair.obj",
		"../obj/fuhua/fuhuaface.obj",
		"../obj/fuhua/fuhuacloak.obj",
	};

	int vertex = 0, face = 0;
	const char* scene_name = "fuhua";
	camera->eye = vec3(0, 1, 2);
	PhongShader *shader_phong = new PhongShader();

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i],0,1);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex,face);
}

void build_xier_scene(Model **model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera)
{
	m = 5;
	const char* modelname[] =
	{
		"../obj/xier/xierbody.obj",
		"../obj/xier/xierhair.obj",
		"../obj/xier/xierface.obj",
		"../obj/xier/xiercloth.obj",
		"../obj/xier/xierarm.obj",
	};
	camera->eye = vec3(0, 1, 2);
	int vertex = 0, face = 0;
	const char* scene_name = "xier";
	PhongShader *shader_phong = new PhongShader();

	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], 0, 1);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}

void build_yayi_scene(Model** model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera)
{
	m = 7;
	const char* modelname[] = {
		"../obj/yayi/yayiface.obj",
		"../obj/yayi/yayibody.obj",
		"../obj/yayi/yayihair.obj",
		"../obj/yayi/yayiarmour1.obj",
		"../obj/yayi/yayiarmour2.obj",
		"../obj/yayi/yayidecoration.obj",
		"../obj/yayi/yayisword.obj"
	};
	camera->eye = vec3(0.65, 1.2, 3);
	int vertex = 0, face = 0;
	const char* scene_name = "yayi";
	PhongShader* shader_phong = new PhongShader();
	
	for (int i = 0; i < m; i++)
	{
		model[i] = new Model(modelname[i], 0, 1);
		vertex += model[i]->nverts();
		face += model[i]->nfaces();
	}

	shader_phong->payload.camera_perp_matrix = perspective;
	shader_phong->payload.camera = camera;

	*shader_use = shader_phong;
	*shader_skybox = NULL;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}
void build_crab_scene(Model** model, int& m, IShader** shader_use, IShader** shader_skybox, mat4 perspective, Camera* camera)
{
	m = 2;
	const char* modelname[] =
	{
		"../obj/crab/crab.obj",
		"../obj/skybox4/box.obj"
	};
	camera->eye = vec3(0, 1, 5);
	PBRShader* shader_pbr = new PBRShader();
	SkyboxShader* shader_sky = new SkyboxShader();

	int vertex = 0, face = 0;
	const char* scene_name = "crab";
	model[0] = new Model(modelname[0], 0, 0); vertex += model[0]->nverts(); face += model[0]->nfaces();
	model[1] = new Model(modelname[1], 1, 0); vertex += model[1]->nverts(); face += model[1]->nfaces();

	shader_pbr->payload.camera_perp_matrix = perspective;
	shader_pbr->payload.camera = camera;
	shader_sky->payload.camera_perp_matrix = perspective;
	shader_sky->payload.camera = camera;

	load_ibl_map(shader_pbr->payload, "../obj/common2");

	*shader_use = shader_pbr;
	*shader_skybox = shader_sky;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}
void build_gun_scene(Model **model, int &m, IShader **shader_use, IShader **shader_skybox, mat4 perspective, Camera *camera)
{
	m = 2;
	const char* modelname[] =
	{
		"../obj/gun/Cerberus.obj",
		"../obj/skybox4/box.obj",
	};
	camera->eye = vec3(0, 1, 5);
	PBRShader *shader_pbr = new PBRShader();
	SkyboxShader *shader_sky = new SkyboxShader();

	int vertex = 0, face = 0;
	const char* scene_name = "gun";
	model[0] = new Model(modelname[0], 0, 0); vertex += model[0]->nverts(); face += model[0]->nfaces();
	model[1] = new Model(modelname[1], 1, 0); vertex += model[1]->nverts(); face += model[1]->nfaces();

	shader_pbr->payload.camera_perp_matrix = perspective;
	shader_pbr->payload.camera = camera;
	shader_sky->payload.camera_perp_matrix = perspective;
	shader_sky->payload.camera = camera;

	load_ibl_map(shader_pbr->payload, "../obj/common2");

	*shader_use = shader_pbr;
	*shader_skybox = shader_sky;

	printf("scene name:%s\n", scene_name);
	printf("model number:%d\n", m);
	printf("vertex:%d faces:%d\n", vertex, face);
}



char* scene::get_text()
{
	return text;
}

extern Camera camera;
void scene::tick(float delta_time)
{
	vec3 camera_pos = camera.get_position();
	vec3 camera_dir = camera.get_forward();

	//vec3 light_pos = light->transform.position;
	//vec3 target_pos = camera.get_target_position();
	//vec3 light_dir = unit_vector(light_pos - target_pos);

	snprintf(text, 500, "");

	char line[50] = "";
	snprintf(line, 50, "camera pos:(%.1f,%.1f,%.1f)\n", camera_pos.x(), camera_pos.y(), camera_pos.z());
	strcat_s(text, line);
	snprintf(line, 50, "camera dir:(%.1f,%.1f,%.1f)\n", TO_DEGREES(camera_dir.x()), TO_DEGREES(camera_dir.y()), TO_DEGREES(camera_dir.z()));
	strcat_s(text, line);

	/*snprintf(line, 50, "press mouse [left] to rotate camera\n");
	strcat(text, line);
	snprintf(line, 50, "press mouse [right] to move camera\n");
	strcat(text, line);*/
	/*snprintf(line, 50, "press key [Space] to reset camera\n\n");
	strcat(text, line);*/

	/*snprintf(line, 50, "light dir: (%.1f,%.1f,%.1f)\n", TO_DEGREES(light_dir.x), TO_DEGREES(light_dir.y), TO_DEGREES(light_dir.z));
	strcat(text, line);
	snprintf(line, 50, "press key [A] or [D] to rotate light dir\n\n");
	strcat(text, line);*/
	//snprintf(line, 50, "press key [E] to switch shadow\n\n");
	strcat_s(text, line);
}
