#ifdef MAKE_DSL
#define ARMATURE_SET 0
#endif

layout (set = ARMATURE_SET, binding = 0) buffer readonly Armature
{
	mat4 bones[];
};
