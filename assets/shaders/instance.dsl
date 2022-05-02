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
