#if defined(ANIM)
layout(binding = 2) uniform ubo_object_animated_
{
	mat4 matrix[8];
}ubo_object;

layout(set = 2, binding = 0) uniform ubo_bone_
{
	mat4 matrix[256];
}ubo_bone[8];
#else
layout(binding = 2) uniform ubo_object_static_
{
	mat4 matrix[1024];
}ubo_object;
#endif
