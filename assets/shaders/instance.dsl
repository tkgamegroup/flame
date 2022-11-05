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

layout (set = SET, binding = 5, r8ui) uniform uimage3D volume_datas[8];

struct MarchingCubesLookupItem
{
	uint    Indices[4];
	uint8_t Vertices[12];
	uint8_t TriangleCount;
	uint8_t VertexCount;
};

layout(set = SET, binding = 6) buffer readonly MarchingCubesLookup
{
	MarchingCubesLookupItem items[256];
}marching_cubes_loopup;
