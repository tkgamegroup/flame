#include "water.pll"

layout(location = 0) out flat uint o_idx;
layout(location = 1) out vec2 o_uv;
layout(location = 2) out vec3 o_coordw;

void main()
{
	uint idx = gl_InstanceIndex >> 16;

	vec2 vs[] = {
		vec2(0.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.0),
		
		vec2(0.0, 1.0),
		vec2(1.0, 1.0),
		vec2(1.0, 0.0)
	};
	vec2 v = vs[gl_VertexIndex];

	o_idx = idx;
	o_uv = v;

	vec2 ext = water_infos[idx].extent;
	o_coordw = water_infos[idx].coord + vec3(ext.x * v.x, 0.0, ext.y * v.y);
	gl_Position = scene.proj_view * vec4(o_coordw, 1.0);
}
