#include "..\math.glsl"

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

const float hf_sqrt2 = 0.7071;

void main()
{
    vec3 eyedir = inversePanorama(inTexcoord);
    
    vec3 color = vec3(0.0);

    if (eyedir.x > hf_sqrt2 && abs(eyedir.y) < hf_sqrt2 && abs(eyedir.z) < hf_sqrt2)
        color = vec3(1.0, 0.0, 0.0);
        
    if (eyedir.y > hf_sqrt2 && abs(eyedir.x) < hf_sqrt2 && abs(eyedir.z) < hf_sqrt2)
        color = vec3(0.0, 1.0, 0.0);
    if (eyedir.z > hf_sqrt2 && abs(eyedir.y) < hf_sqrt2 && abs(eyedir.x) < hf_sqrt2)
        color = vec3(0.0, 0.0, 1.0);
    if (eyedir.x < -hf_sqrt2 && abs(eyedir.y) < hf_sqrt2 && abs(eyedir.z) < hf_sqrt2)
        color = vec3(0.2, 0.0, 0.0);
    if (eyedir.y < -hf_sqrt2 && abs(eyedir.x) < hf_sqrt2 && abs(eyedir.z) < hf_sqrt2)
        color = vec3(0.0, 0.2, 0.0);
    if (eyedir.z < -hf_sqrt2 && abs(eyedir.y) < hf_sqrt2 && abs(eyedir.x) < hf_sqrt2)
        color = vec3(0.0, 0.0, 0.2);
        

    outColor = vec4(color, 1.0);
}
