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
	uint height_map;
};

layout(set = SET, binding = 2) buffer readonly TerrainInstances
{
	TerrainInstance terrain_instances[8];
};
