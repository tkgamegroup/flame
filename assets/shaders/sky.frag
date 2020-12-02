#include "sky.pll"
#include "math.glsl"

layout (location = 0) in vec3 i_dir;

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = texture(sky_box, normalize(i_dir));
}
