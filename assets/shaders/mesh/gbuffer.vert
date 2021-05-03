#include "gbuffer.pll"

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;

layout (location = 0) out flat uint o_mat;
layout (location = 1) out vec2 o_uv;
layout (location = 2) out vec3 o_normal;

void main()
{
	uint mod_idx = gl_InstanceIndex >> 16;
	o_mat = gl_InstanceIndex & 0xffff;
	o_uv = i_uv;

	vec3 coordw = vec3(transforms[mod_idx].mat * vec4(i_position, 1.0));
	o_normal = mat3(transforms[mod_idx].nor) * i_normal;
	gl_Position = render_data.proj_view * vec4(coordw, 1.0);
}
