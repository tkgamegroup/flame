#include "sky.pll"
#include "math.glsl"

layout (location = 0) in vec2 i_uv;
layout (location = 1) in vec3 i_dir;

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = texture(maps[render_data.sky_tex_id], panorama(normalize(i_dir)));
	o_color = vec4(panorama(normalize(i_dir)), 0.0, 1.0);
	o_color = vec4(vec3(dot(normalize(i_dir), vec3(0, 1, 0))), 1.0);
	o_color = vec4(normalize(i_dir), 1.0);
	//o_color = vec4(normalize(i_dir), 1.0);
}
