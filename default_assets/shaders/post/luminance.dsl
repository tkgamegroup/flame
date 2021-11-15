#ifdef MAKE_DSL
#define LUMINANCE_SET 0
#endif

layout (set = LUMINANCE_SET, rgba16f, binding = 0) uniform readonly image2D img_col;

layout (set = LUMINANCE_SET, binding = 1) buffer Histogram
{
    uint histogram[256];
};

layout (set = LUMINANCE_SET, binding = 2) buffer AverageLum
{
    float average_lum;
};
