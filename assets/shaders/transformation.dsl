#ifdef MAKE_DSL
#define TRANSFORMATION_SET 0
#endif

struct Transformation
{
	mat4 mat;
	mat4 nor;
};

layout (set = TRANSFORMATION_SET, binding = 0) buffer readonly Transformations
{
	Transformation transformations[];
};
