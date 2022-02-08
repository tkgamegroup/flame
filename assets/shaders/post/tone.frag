#include "../math.glsl"
#include "tone.pll"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

float reinhard2(float x, float L_white) 
{
    return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
}

void main()
{
	vec3 rgb = texture(image, i_uv).rgb;
	vec3 Yxy = rgb2Yxy(rgb);
	float lp = Yxy.x / (9.6 * average_lum + 0.0001);
	Yxy.x = reinhard2(lp, pc.white_point);
	rgb = Yxy2rgb(Yxy);
	o_color = vec4(pow(rgb, vec3(pc.one_over_gamma)), 1.0);
}
