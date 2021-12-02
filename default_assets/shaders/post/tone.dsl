layout (set = SET, binding = 0) uniform sampler2D image;

layout (set = SET, binding = 1) buffer AverageLum
{
    float average_lum;
};
