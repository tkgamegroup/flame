layout(quads, equal_spacing, ccw) in;

layout(location = 0) in vec2 i_uvs[];

layout(location = 0) out vec2 o_uv;
#ifndef DEPTH_ONLY
layout(location = 1) out vec3 o_normal;
layout(location = 2) out vec3 o_tangent;
layout(location = 3) out vec3 o_coordw;
#endif

void main()
{
	uint terrain_id = pc.index & 0xffff;

	o_uv = mix(
		mix(i_uvs[0], i_uvs[1], gl_TessCoord.x),
		mix(i_uvs[3], i_uvs[2], gl_TessCoord.x),
		gl_TessCoord.y
	);

	vec3 world_pos = vec3(mix(
		mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
		mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x),
		gl_TessCoord.y
	));
	world_pos.y += texture(terrain_height_maps[terrain_id], o_uv).r * instance.terrains[terrain_id].extent.y;
	
#ifndef DEPTH_ONLY
	o_coordw = world_pos;
	o_normal = normalize(texture(terrain_normal_maps[terrain_id], o_uv).xyz * 2.0 - 1.0);
	o_tangent = normalize(texture(terrain_tangent_maps[terrain_id], o_uv).xyz * 2.0 - 1.0);
#endif
#ifndef HAS_GEOM
	#ifndef DEPTH_ONLY
		gl_Position = camera.proj_view * vec4(world_pos, 1.0);
	#else
		if (pc.i[0] == 0)
			gl_Position = lighting.dir_shadows[pc.i[1]].mats[pc.i[2]] * vec4(world_pos, 1.0);
		else
			gl_Position = lighting.pt_shadows[pc.i[1]].mats[pc.i[2]] * vec4(world_pos, 1.0);
	#endif
#else
	gl_Position = vec4(world_pos, 1.0);
#endif
}
