#include "sky.pll"

layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec3 o_dir;

void main()
{
	vec2 vs[] = {
		vec2(0.0, 0.0),
		vec2(0.0, 2.0),
		vec2(2.0, 0.0)
	};
	vec2 v = vs[gl_VertexIndex];
	o_uv = v;
	v = v * 2.0 - 1.0;
	o_dir = mat3(render_data.camera_dirs) * normalize(vec3(-v.x, v.y, 1.0));
	gl_Position = vec4(v, 1.0, 1.0);
}
