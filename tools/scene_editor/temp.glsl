#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define SET 0

#define DEFERRED
#define DEPTH_PASS
layout (set = SET, binding = 0) uniform Scene
{
	float sky_intensity;
	float sky_rad_levels;
	vec3 fog_color;

	float zNear;
	float zFar;

	vec2 viewport;
	
	vec3 camera_coord;
	vec3 camera_dir;

	mat4 view;
	mat4 view_inv;
	mat4 proj;
	mat4 proj_inv;
	mat4 proj_view;
	mat4 proj_view_inv;
	
	vec4 frustum_planes[6];

	uint time;
}scene;


#undef SET
#define SET 1

struct MeshInstance
{
	mat4 mat;
	mat4 nor;
};

layout(set = SET, binding = 0) buffer readonly MeshInstances
{
	MeshInstance mesh_instances[65536];
};

struct ArmatureInstance
{
	mat4 bones[128];
};

layout(set = SET, binding = 1) buffer readonly ArmatureInstances
{
	ArmatureInstance armature_instances[256];
};

struct TerrainInstance
{
	mat4 mat;
	vec3 extent;
	uvec2 blocks;
	uint tess_level;
};

layout(set = SET, binding = 2) buffer readonly TerrainInstances
{
	TerrainInstance terrain_instances[8];
};

layout (set = SET, binding = 3) uniform sampler2D terrain_height_maps[8];
layout (set = SET, binding = 4) uniform sampler2D terrain_normal_maps[8];
layout (set = SET, binding = 5) uniform sampler2D terrain_tangent_maps[8];
layout (set = SET, binding = 6) uniform sampler2D terrain_splash_maps[8];


#undef SET
#define SET 2

struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	float alpha_test;
	int opaque;

	vec4 f;
	ivec4 i;

	int map_indices[8];
};

layout (set = SET, binding = 0) uniform MaterialMisc
{
	int black_map_id;
	int white_map_id;
	int random_map_id;
}material_misc;

layout (set = SET, binding = 1) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[128];
};

layout (set = SET, binding = 2) uniform sampler2D material_maps[128];


#undef SET
#define SET 3




layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_nor;
//layout (location = 5) in vec3 i_tan;
//layout (location = 6) in vec3 i_bit;

layout(location = 0) out flat uint o_matid;
layout(location = 1) out vec2 o_uv;

void main()
{
	uint id = gl_InstanceIndex >> 8;
	o_matid = gl_InstanceIndex & 0xff;
	o_uv = i_uv;

	o_coordw = vec3(mesh_instances[id].mat * vec4(i_pos, 1.0));

	if (pc.i[0] == 0)
		gl_Position = dir_shadows[pc.i[1]].mats[pc.i[2]] * vec4(o_coordw, 1.0);
	else
		gl_Position = pt_shadows[pc.i[1]].mats[pc.i[2]] * vec4(o_coordw, 1.0);
}


