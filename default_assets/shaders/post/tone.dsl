#ifdef MAKE_DSL
#define TONE_SET 0
#endif

layout (set = TONE_SET, binding = 0) uniform sampler2D image;

layout (set = TONE_SET, binding = 1) buffer AverageLum
{
    float average_lum;
};
