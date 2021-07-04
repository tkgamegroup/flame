#include "particle.pll"

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec4 i_color;

layout(location = 0) out vec4 o_color;
layout(location = 1) out vec2 o_uv;
layout(location = 2) out flat uint o_id;

void main()
{
	o_color = i_color;
	o_uv = i_uv;
	o_id = gl_InstanceIndex;
	gl_Position = render_data.proj_view * vec4(i_position, 1.0);
}
