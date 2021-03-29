#ifdef MAKE_DSL
#define TRANSFORM_SET 0
#endif

struct Transform
{
	mat4 mat;
	mat4 nor;
};

layout (set = TRANSFORM_SET, binding = 0) buffer readonly Transforms
{
	Transform transforms[];
};
