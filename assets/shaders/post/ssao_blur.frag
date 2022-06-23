layout (location = 0) in vec2 i_uv;

layout (location = 0) out float o_result;

void main() 
{
    float res = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * pc.pxsz;
            res += texture(image, i_uv + offset).r;
        }
    }
    o_result = res / (4.0 * 4.0);
}
