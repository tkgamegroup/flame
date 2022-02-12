#ifndef DEFERRED
#include "..\forward.pll"
#else
#include "..\gbuffer.pll"
#endif

layout(quads, equal_spacing, ccw) in;

layout(location = 0) in flat uint i_ids[];
layout(location = 1) in flat uint i_matids[];
layout(location = 2) in		 vec2 i_uvs[];

layout(location = 0) out flat uint o_id;
layout(location = 1) out flat uint o_matid;
layout(location = 2) out vec2 o_uv;
layout(location = 3) out vec3 o_normal;
layout(location = 4) out vec3 o_tangent;
#ifndef DEFERRED
layout(location = 5) out vec3 o_coordw;
#endif

void main()
{
	uint id = i_ids[0];
	TerrainInstance terrain = terrain_instances[id];

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
	coordw.y += texture(terrain_textures[id], vec3(uv, 0)).r * terrain.extent.y;

	o_id = id;
	o_matid = i_matids[0];
	o_uv = uv;

	o_normal = normalize(texture(terrain_textures[id], vec3(uv, 1)).xyz * 2.0 - 1.0);
	o_tangent = normalize(texture(terrain_textures[id], vec3(uv, 2)).xyz * 2.0 - 1.0);

#ifndef DEFERRED
	o_coordw = coordw;
#endif

	gl_Position = scene.proj_view * vec4(coordw, 1.0);
}
