#version 460
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_shader_explicit_arithmetic_types: require
#extension GL_EXT_shader_explicit_arithmetic_types_int8: require

#define BLACK_TEX_ID 1
#define COLOR_MAP -2
#define EMISSIVE_MAP 1
#define GBUFFER_PASS
#define MAT_CODE D:\flame\assets\shaders\default_mat.glsl
#define RAND_TEX_ID 2
#define WHITE_TEX_ID 0


#define SET 0

layout (set = SET, binding = 0) uniform Camera
{
	float zNear;
	float zFar;
	float fovy;
	float tan_hf_fovy;

	vec2 viewport;
	
	vec3 coord;
	vec3 front;
	vec3 right;
	vec3 up;

	mat4 view;
	mat4 view_inv;
	mat4 last_view;
	mat4 proj;
	mat4 proj_inv;
	mat4 proj_view;
	mat4 proj_view_inv;
	
	vec4 frustum_planes[6];

	uint time;
}camera;


#undef SET
#define SET 1

struct MeshInstance
{
	mat4 mat;
	mat4 nor;
};

struct ArmatureInstance
{
	mat4 bones[128];
};

struct TerrainInstance
{
	mat4 mat;
	vec3 extent;
	uvec2 blocks;
	uint tess_level;
	uint grass_field_tess_level;
	uint grass_channel;
	int grass_texture_id;
};

struct SdSphere
{
	vec3 coord;
	float radius;
};

struct SdBox
{
	vec3 coord;
	vec3 extent;
};

struct SdfInstance
{
	uint boxes_count;
	SdBox boxes[64];
	uint spheres_count;
	SdSphere spheres[64];
};

struct VolumeInstance
{
	mat4 mat;
	vec3 extent;
	uvec3 blocks;
};

layout(set = SET, binding = 0) buffer readonly Instance
{
	MeshInstance meshes[65536];
	ArmatureInstance armatures[256];
	TerrainInstance terrains[8];
	SdfInstance sdfs[256];
	VolumeInstance volumes[16];
}instance;

layout (set = SET, binding = 1) uniform sampler2D terrain_height_maps[8];
layout (set = SET, binding = 2) uniform sampler2D terrain_normal_maps[8];
layout (set = SET, binding = 3) uniform sampler2D terrain_tangent_maps[8];
layout (set = SET, binding = 4) uniform sampler2D terrain_splash_maps[8];

layout (set = SET, binding = 5) uniform sampler3D volume_data_maps[8];
layout (set = SET, binding = 6) uniform sampler3D volume_splash_maps[8];

struct MarchingCubesLookupItem
{
	uint8_t Vertices[15];
	uint8_t TriangleCount;
};

layout(set = SET, binding = 7) buffer readonly MarchingCubesLookup
{
	MarchingCubesLookupItem items[256];
}marching_cubes_loopup;

layout(set = SET, binding = 8) buffer TransformFeedback
{
	uint vertex_count;
	float vertex_x[524288];
	float vertex_y[524288];
	float vertex_z[524288];
}transform_feedback;


#undef SET
#define SET 2

struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	vec4 emissive;
	float normal_map_strength;
	float emissive_map_strength;
	uint flags;

	vec4 f;
	ivec4 i;

	int map_indices[8];
};

layout (set = SET, binding = 0) buffer readonly Material
{
	vec4 vars[128];
	MaterialInfo infos[128];
}material;

layout (set = SET, binding = 1) uniform sampler2D material_maps[128];

vec4 mat_var(uint id)
{
	return material.vars[id];
}

vec4 sample_map(uint id, in vec2 uv)
{
	return texture(material_maps[id], uv);
}


#undef SET
#define SET 3


layout (push_constant) uniform PushConstant
{
	vec4 f;
	ivec4 i;
	uint index;
	vec3 offset;
}pc;


const float PI = 3.14159265359;
const float PI_INV = 1.0 / PI;

float sum(vec3 v) 
{ 
	return v.x + v.y + v.z;
}

float length_squared(vec2 v)
{
	return v.x * v.x + v.y * v.y;
}

float length_squared(vec3 v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand(vec3 co)
{
    return fract(sin(dot(co, vec3(12.9898, 78.233, 53.539))) * 43758.5453);
}

vec2 panorama(vec3 v)
{
	return vec2(0.5 + 0.5 * atan(v.x, v.z) * PI_INV, acos(v.y) * PI_INV);
}

vec4 pack_uint_to_v4(uint id)
{
	vec4 ret;
	ret[0] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[1] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[2] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[3] = (id & 0xff) / 255.0;
	return ret;
}

float linear_depth(float near, float far, float d /* -1, +1 */)
{
	return 2.0 * near * far / (far + near - d * (far - near));
}

float sd_circle(vec2 p, float r)
{
	return length(p) - r;
}

float sd_ori_box(vec2 p, vec2 a, vec2 b, float th)
{
	float l = length(b - a);
	vec2 d = (b - a) / l;
	vec2 q = (p - (a + b) * 0.5);
	q = mat2(d.x, -d.y, d.y, d.x) * q;
	q = abs(q) - vec2(l, th) * 0.5;
	return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}

float sd_sphere(vec3 p, float s) 
{
    return length(p) - s;
}

float ud_box(vec3 p, vec3 b) 
{
    return length(max(abs(p) - b, 0.0));
}

float ud_round_box(vec3 p, vec3 b, float r) 
{
    return length(max(abs(p) - b, 0.0)) - r;
}

float op_smooth_union( float d1, float d2, float k )
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

float op_smooth_subtraction( float d1, float d2, float k )
{
    float h = max(k-abs(-d1-d2),0.0);
    return max(-d1, d2) + h*h*0.25/k;
}

float interpolate(float v, float a, float b, float t)
{
	float a0 = a - t * 0.5;
	float a1 = a + t * 0.5;
	float b0 = b - t * 0.5;
	float b1 = b + t * 0.5;
	if (v <= a0 || v >= b1)
		return 0.f;
	if (v >= a1 && v <= b0)
		return 1.f;
	if (v < a1)
	{
		if (a0 < 0)
			return 1.f;
		return 1.f - (a1 - v) / t;
	}
	else if (v > b0)
	{
		if (b1 > 1)
			return 1.f;
		return 1.f - (v - b0) / t;
	}
	return 0.f;
}

vec3 cube_coord(vec3 v)
{
	return vec3(v.x, v.y, v.z);
}

mat3 rotation(vec3 axis, float angle)
{
	float c = cos(angle);
	float s = sin(angle);

	vec3 temp = (1.0 - c) * axis;

	mat3 ret;
	ret[0][0] = c + temp[0] * axis[0];
	ret[0][1] = temp[0] * axis[1] + s * axis[2];
	ret[0][2] = temp[0] * axis[2] - s * axis[1];

	ret[1][0] = temp[1] * axis[0] - s * axis[2];
	ret[1][1] = c + temp[1] * axis[1];
	ret[1][2] = temp[1] * axis[2] + s * axis[0];

	ret[2][0] = temp[2] * axis[0] + s * axis[1];
	ret[2][1] = temp[2] * axis[1] - s * axis[0];
	ret[2][2] = c + temp[2] * axis[2];
	return ret;
}

vec3 rgb2xyz(vec3 _rgb)
{
	vec3 xyz;
	xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), _rgb);
	xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
	xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), _rgb);
	return xyz;
}

vec3 xyz2rgb(vec3 _xyz)
{
	vec3 rgb;
	rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
	rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
	rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
	return rgb;
}

vec3 xyz2Yxy(vec3 _xyz)
{
	float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
	return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
}

vec3 Yxy2xyz(vec3 _Yxy)
{
	vec3 xyz;
	xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
	xyz.y = _Yxy.x;
	xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
	return xyz;
}

vec3 rgb2Yxy(vec3 _rgb)
{
	return xyz2Yxy(rgb2xyz(_rgb) );
}

vec3 Yxy2rgb(vec3 _Yxy)
{
	return xyz2rgb(Yxy2xyz(_Yxy) );
}




layout(location = 0) in flat uint i_matid;
layout(location = 1) in	     vec2 i_uv;
layout(location = 2) in      vec3 i_normal;
layout(location = 3) in      vec3 i_tangent;
layout(location = 4) in      vec3 i_coordw;

layout(location = 0) out vec4 o_gbufferA;
layout(location = 1) out vec4 o_gbufferB;
layout(location = 2) out vec4 o_gbufferC;
layout(location = 3) out vec4 o_gbufferD;

void main()
{
	MaterialInfo material = material.infos[i_matid];
			vec4 color = sample_map(material.map_indices[COLOR_MAP], i_uv);


	float metallic;
		metallic = material.metallic;

	float roughness;
		roughness = material.roughness;

	vec3 N = i_normal;

	vec3 emissive;
		emissive = sample_map(material.map_indices[EMISSIVE_MAP], i_uv).rgb;
		emissive *= material.emissive_map_strength;
		
		o_gbufferA = vec4(color.rgb, 0.0);
		o_gbufferB = vec4(N * 0.5 + 0.5, 0.0);
		o_gbufferC = vec4(metallic, roughness, 0.0, material.flags / 255.0);
		o_gbufferD = vec4(emissive, 0.0);


}


