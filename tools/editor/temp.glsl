#version 460
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_shader_explicit_arithmetic_types: require
#extension GL_EXT_shader_explicit_arithmetic_types_int8: require
#extension GL_EXT_mesh_shader : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_shuffle_relative : require



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

	float time;
}camera;


#undef SET
#define SET 1

struct MeshInstance
{
	mat4 mat;
	mat3 nor;
	uint col;
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

layout (set = SET, binding = 4) uniform sampler3D volume_data_maps[8];

struct MarchingCubesLookupItem
{
	uint8_t Vertices[15];
	uint8_t TriangleCount;
};

layout(set = SET, binding = 5) buffer readonly MarchingCubesLookup
{
	MarchingCubesLookupItem items[256];
}marching_cubes_loopup;

layout(set = SET, binding = 6) buffer TransformFeedback
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
	float tiling;
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
	uint cel_shading_levels;
	uint csm_levels;
	float esm_factor;
	float shadow_bleeding_reduction;
	float shadow_darkening;
	int fog_type;
	float fog_density;
	float fog_start;
	float fog_end;
	float fog_base_height;
	float fog_max_height;
	vec3 fog_color;
	uint ssr_enable;
	float ssr_thickness;
	float ssr_max_distance;
	int ssr_max_steps;
	int ssr_binary_search_steps;
	
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

layout(set = SET, binding = 0) uniform sampler2D img_dep;
layout(set = SET, binding = 1) uniform sampler2D img_last_dst;
layout(set = SET, binding = 2) uniform sampler2D img_last_dep;


#undef SET
#define SET 5


layout (push_constant) uniform PushConstant
{
	vec4 f;
	ivec4 i;
	uint index;
	vec3 offset;
}pc;


/* * * * * * * * * * * * * Author's note * * * * * * * * * * * *\
*   _       _   _       _   _       _   _       _     _ _ _ _   *
*  |_|     |_| |_|     |_| |_|_   _|_| |_|     |_|  _|_|_|_|_|  *
*  |_|_ _ _|_| |_|     |_| |_|_|_|_|_| |_|     |_| |_|_ _ _     *
*  |_|_|_|_|_| |_|     |_| |_| |_| |_| |_|     |_|   |_|_|_|_   *
*  |_|     |_| |_|_ _ _|_| |_|     |_| |_|_ _ _|_|  _ _ _ _|_|  *
*  |_|     |_|   |_|_|_|   |_|     |_|   |_|_|_|   |_|_|_|_|    *
*                                                               *
*                     http://www.humus.name                     *
*                                                                *
* This file is a part of the work done by Humus. You are free to   *
* use the code in any way you like, modified, unmodified or copied   *
* into your own work. However, I expect you to respect these points:  *
*  - If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  - For use in anything commercial, please request my approval.     *
*  - Share your work and ideas too as much as you can.             *
*                                                                *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// 128x128x128 cubes by default. (1 << 7) == 128
#define SHIFT 7
#define GRID_SIZE (1 << SHIFT)
#define STEP_SIZE (1.0f / float(GRID_SIZE))

struct Task
{
	uint meshlets[64];
};

uint volume_id = pc.index & 0xffff;
vec3 extent = instance.volumes[volume_id].extent;
vec3 blocks = vec3(instance.volumes[volume_id].blocks);
#define DATA_MAP volume_data_maps[volume_id]

vec3 block_sz = vec3(1.0) / blocks;
vec3 block_off = block_sz * pc.offset;

float field(vec3 pos)
{
	return texture(DATA_MAP, pos * block_sz + block_off).r * 2.0 - 1.0;
}



taskPayloadSharedEXT Task payload;

shared uint values[5][5][5]; // 125 intermediate values for the 5x5x5 corners of 4x4x4 cubes
shared uint fetch_values[3][3][3]; // 3x3x3 samples, fetching 6x6x6 values of which we use 5x5x5

layout (local_size_x = 32) in;
void main()
{
	uint thread_id = gl_LocalInvocationID.x;
	uint workgroup_id = gl_WorkGroupID.x;

	// Convert linear meshlet_id into x, y and z coordinates. We process a 4x4x4 volume of cubes per task shader invocation. For that we need to evaluate 5x5x5 field function values.
	uint mi_x = 4 * ((workgroup_id >> (0*(SHIFT-2))) & (GRID_SIZE/4 - 1));
	uint mi_y = 4 * ((workgroup_id >> (1*(SHIFT-2))) & (GRID_SIZE/4 - 1));
	uint mi_z = 4 * ((workgroup_id >> (2*(SHIFT-2))));

	// 125 values are needed, so this loops 4 times, 3 lanes get wasted on last iteration
	for (uint i = thread_id; i < 5 * 5 * 5; i += 32)
	{
		uint t = (205 * i) >> 10; // Fast i / 5, works for values < 1024. (205/1024 = 0.2001953125 ~ 1/5)
		uint x = i - 5 * t;       // Fast i % 5
		uint z = (205 * t) >> 10;
		uint y = t - 5 * z;

		vec3 pos = vec3(mi_x + x, mi_y + y, mi_z + z) * STEP_SIZE;
		values[z][y][x] = (field(pos) >= 0.0f)? 1 : 0;
	}

	// Two loops, all lanes used
	uint count = 0;
	for (uint i = thread_id; i < 64; i += 32)
	{
		uint x = i & 0x3;
		uint y = (i >> 2) & 0x3;
		uint z = i >> 4;

		// Collect the sign bits for the cube corners. If all are zeros or all ones we're either fully inside or outside
		// the surface, so no triangles will be generated. In all other cases, the isosurface cuts through the cube somewhere.
		uint cube_index;
		cube_index  = (values[z + 0][y + 0][x + 0] << 0);
		cube_index |= (values[z + 0][y + 0][x + 1] << 1);
		cube_index |= (values[z + 0][y + 1][x + 0] << 2);
		cube_index |= (values[z + 0][y + 1][x + 1] << 3);
		cube_index |= (values[z + 1][y + 0][x + 0] << 4);
		cube_index |= (values[z + 1][y + 0][x + 1] << 5);
		cube_index |= (values[z + 1][y + 1][x + 0] << 6);
		cube_index |= (values[z + 1][y + 1][x + 1] << 7);

		// See if our cube intersects the isosurface
		bool accept = (cube_index != 0 && cube_index != 0xFF);

		// Gather which lanes have an intersected cube and get the index where to put ours (if we have a valid one)
		uvec4 ballot = subgroupBallot(accept);

		if (accept)
		{
			uint index = bitCount(ballot.x & ((1 << thread_id) - 1));

			// Output a linear meshlet ID for the mesh shader
			uint meshlet_id = ((((mi_z + z) << SHIFT) + mi_y + y) << SHIFT) + mi_x + x;
			payload.meshlets[count + index] = meshlet_id;
		}

		count += bitCount(ballot.x);
	}

	if (thread_id == 0)
		EmitMeshTasksEXT(count, 1, 1);
}


