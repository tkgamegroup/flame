layout (set = SET, rgba16f, binding = 0) uniform readonly image2D img_col;

layout (set = SET, binding = 1) buffer Histogram
{
    uint histogram[256];
};

layout (set = SET, binding = 2) buffer AverageLum
{
    float average_lum;
};
