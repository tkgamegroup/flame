#include "terrain.pll"

layout(quads, equal_spacing, ccw) in;

layout (location = 0) in flat uint i_idxs[];
layout (location = 1) in vec2 i_uvs[];
 
layout (location = 0) out flat uint o_idx;
layout (location = 1) out vec2 o_uv;
layout (location = 2) out vec3 o_coordw;
layout (location = 3) out vec3 o_coordv;
layout (location = 4) out vec3 o_normal;

vec3 normalx(vec3 v)
{
	return normalize(vec3(-v.y, v.x, 0));
}

vec3 normalz(vec3 v)
{
	return normalize(vec3(0, -v.z, v.y));
}

void main()
{
	uint idx = i_idxs[0];
	TerrainInfo terrain = terrain_infos[idx];

	vec2 uv = mix(
		mix(i_uvs[0], i_uvs[1], gl_TessCoord.x), 
		mix(i_uvs[3], i_uvs[2], gl_TessCoord.x), 
		gl_TessCoord.y
	);
	
	vec3 p = vec3(mix(
		mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x), 
		mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x), 
		gl_TessCoord.y
	));
	p.y += texture(maps[terrain.height_tex_id], uv).r * terrain.scale.y;

	o_idx = idx;
	o_uv = uv;
	o_coordw = p;
	o_coordv = render_data.camera_coord - p;
	vec3 n = texture(maps[terrain.normal_tex_id], uv).xyz * 2.0 - vec3(1.0);
	o_normal = vec3(n.x, n.z, -n.y);

	gl_Position = render_data.proj_view * vec4(p, 1.0);
}
