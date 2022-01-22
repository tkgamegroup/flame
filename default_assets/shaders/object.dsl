struct Object
{
	mat4 mat;
	mat4 nor;
};

layout(set = SET, binding = 0) buffer readonly Objects
{
	Object objects[65536];
};

struct Armature
{
	mat4 bones[128];
};

layout(set = SET, binding = 1) buffer readonly Armatures
{
	Armature armatures[128];
};
