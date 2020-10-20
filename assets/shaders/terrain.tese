#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define RENDER_DATA_SET 0
#define MATERIAL_SET 1
#define TERRAIN_SET 3

#include "render_data_dsl.glsl"
#include "material_dsl.glsl"
#include "terrain_dsl.glsl"

layout(quads, equal_spacing, ccw) in;

layout (location = 0) in flat uint i_idxs[];
layout (location = 1) in vec2 i_uvs[];
layout (location = 2) in vec4 i_debug[];
 
layout (location = 0) out flat uint o_idx;
layout (location = 1) out vec2 o_uv;
layout (location = 2) out vec3 o_coordw;
layout (location = 3) out vec3 o_coordv;
layout (location = 4) out vec3 o_normal;
layout (location = 5) out vec4 o_debug;

TerrainInfo terrain;
vec2 uv;
uint id;

vec3 get_coord(vec2 off)
{
	vec3 ret = vec3(mix(
		mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x + off.x), 
		mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x + off.x), 
		gl_TessCoord.y + off.y
	));
	//ret.y += 0.105882352 * terrain.scale.y;
	ret.y += texture(maps[id], uv + off / terrain.blocks).r * terrain.scale.y;
	return ret;
}

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
	terrain = terrain_infos[idx];

	id = terrain.height_tex_id;

	uv = mix(
		mix(i_uvs[0], i_uvs[1], gl_TessCoord.x), 
		mix(i_uvs[3], i_uvs[2], gl_TessCoord.x), 
		gl_TessCoord.y
	);

	vec3 p = get_coord(vec2(0));

	gl_Position = render_data.proj_view * vec4(p, 1.0);

	o_idx = idx;
	o_uv = uv;
	o_coordw = p;
	o_coordv = render_data.camera_coord - p;

	float off = 1.0 / terrain.tess_levels;
	vec3 n0 = normalx(get_coord(vec2(off, 0)) - p);
	vec3 n1 = normalx(p - get_coord(vec2(-off, 0)));
	vec3 n2 = normalz(get_coord(vec2(0, -off)));
	vec3 n3 = normalz(p - get_coord(vec2(0, off)));
	o_normal = (n0 + n1 + n2 + n3) * 0.25;
}
