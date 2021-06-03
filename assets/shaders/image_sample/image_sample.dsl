#ifdef MAKE_DSL
#define IMAGE_SAMPLE_SET 0
#endif

layout (set = IMAGE_SAMPLE_SET, binding = 0) uniform sampler2D tex;

layout (set = IMAGE_SAMPLE_SET, binding = 1) buffer writeonly Results
{
	vec4 results[];
};
