#include "sky.pll"

layout (location = 0) in vec3 i_position;

layout (location = 0) out vec3 o_dir;

void main()
{
	o_dir = i_position;
	gl_Position = render_data.proj_view * vec4(i_position, 0.0);
}
