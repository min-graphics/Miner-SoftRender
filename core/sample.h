#pragma once
#include "../shader/shader.h"
#include <stdlib.h>
#include <thread>

vec3 cubemap_sampling(vec3 direction, cubemap_t* cubemap);
vec3 texture_sample(vec2 uv, TGAImage* image);

//下面这两个函数是用来生成IBL使用的预滤波环境贴图和辐照度贴图的
//void generate_prefilter_map(int thread_id, int face_id, int mip_level, TGAImage& image);
//void generate_irradiance_map(int thread_id, int face_id, TGAImage &image);