#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define BLACK_TEX_ID 1
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

layout(set = SET, binding = 0) buffer readonly Instance
{
	MeshInstance meshes[65536];
	ArmatureInstance armatures[256];
	TerrainInstance terrains[8];
	SdfInstance sdfs[256];
}instance;

layout (set = SET, binding = 1) uniform sampler2D terrain_height_maps[8];
layout (set = SET, binding = 2) uniform sampler2D terrain_normal_maps[8];
layout (set = SET, binding = 3) uniform sampler2D terrain_tangent_maps[8];
layout (set = SET, binding = 4) uniform sampler2D terrain_splash_maps[8];


#undef SET
#define SET 2

struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	int opaque;

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

struct DirLight
{
	vec3 dir;
	vec3 color;
	int shadow_index;
};

struct PtLight
{
	vec3 pos;
	vec3 color;
	int shadow_index;
};

struct DirShadow
{
	mat4 mats[4];
	vec4 splits;
	float far;
};

struct PtShadow
{
	mat4 mats[6];
	float near;
	float far;
};

layout (set = SET, binding = 0) buffer readonly Lighting
{
	float sky_intensity;
	float sky_rad_levels;
	vec3 fog_color;
	
	DirLight dir_lights[4];
	uint dir_lights_count;
	uint dir_lights_list[4];
	DirShadow dir_shadows[4];

	PtLight pt_lights[1024];
	uint pt_lights_count;
	uint pt_lights_list[1024];
	PtShadow pt_shadows[4];
}lighting;

layout (set = SET, binding = 1) uniform sampler2DArray	dir_shadow_maps[4];
layout (set = SET, binding = 2) uniform samplerCube		pt_shadow_maps[4];

layout(set = SET, binding = 3) uniform samplerCube sky_map;
layout(set = SET, binding = 4) uniform samplerCube sky_irr_map;
layout(set = SET, binding = 5) uniform samplerCube sky_rad_map;
layout(set = SET, binding = 6) uniform sampler2D brdf_map;


#undef SET
#define SET 4


layout (push_constant) uniform PushConstant
{
	vec4 f;
	ivec4 i;
}pc;



layout(location = 0) in vec4 i_col;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in flat uint i_matid;

layout(location = 0) out vec4 o_color;

void main()
{
	MaterialInfo material = material.infos[i_matid];
		vec4 color = material.color;
		
	float metallic = material.metallic;
	float roughness = material.roughness;
	
		vec3 albedo = (1.0 - metallic) * color.rgb;
		vec3 f0 = mix(vec3(0.04), color.rgb, metallic);
		o_color = vec4(shading(i_coordw, i_normal, metallic, albedo, f0, roughness, 1.0), color.a);


}


