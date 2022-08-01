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
	int grass_field_id;
};

layout(set = SET, binding = 2) buffer readonly TerrainInstances
{
	TerrainInstance terrain_instances[8];
};

struct GrassField
{
	uint tess_level;
	int texture_id;
};

layout(set = SET, binding = 3) buffer readonly GrassFieldInstances
{
	GrassField grass_field_instances[8];
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

layout(set = SET, binding = 4) buffer readonly SdfInstances
{
	SdfInstance sdf_instances[256];
};

layout (set = SET, binding = 5) uniform sampler2D terrain_height_maps[8];
layout (set = SET, binding = 6) uniform sampler2D terrain_normal_maps[8];
layout (set = SET, binding = 7) uniform sampler2D terrain_tangent_maps[8];
layout (set = SET, binding = 8) uniform sampler2D terrain_splash_maps[8];
