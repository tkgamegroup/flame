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
	ArmatureInstance armature_instances[128];
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

// height, normal, tangent
layout (set = SET, binding = 3) uniform sampler2DArray terrain_textures[8];