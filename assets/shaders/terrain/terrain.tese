#ifndef DEFERRED
#include "forward.pll"
#else
#include "gbuffer.pll"
#endif

layout(quads, equal_spacing, ccw) in;

layout(location = 0) in flat uint i_idxs[];
layout(location = 1) in vec2 i_uvs[];

layout(location = 0) out flat uint o_idx;
layout(location = 1) out vec2 o_uv;
layout(location = 2) out vec3 o_normal;
#ifndef DEFERRED
layout(location = 3) out vec3 o_coordw;
layout(location = 4) out vec3 o_coordv;
#endif

void main()
{
	uint idx = i_idxs[0];
	TerrainInfo terrain = terrain_infos[idx];

	vec2 uv = mix(
		mix(i_uvs[0], i_uvs[1], gl_TessCoord.x),
		mix(i_uvs[3], i_uvs[2], gl_TessCoord.x),
		gl_TessCoord.y
	);

	vec3 coordw = vec3(mix(
		mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
		mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x),
		gl_TessCoord.y
	));
	coordw.y += texture(maps[terrain.height_map_id], uv).r * terrain.extent.y;

	o_idx = idx;
	o_uv = uv;

	o_normal = texture(maps[terrain.normal_map_id], uv).xyz * 2.0 - vec3(1.0);

#ifndef DEFERRED
	o_coordw = coordw;
	o_coordv = render_data.camera_coord - coordw;
#endif

	gl_Position = render_data.proj_view * vec4(coordw, 1.0);
}
