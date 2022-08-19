layout(quads, equal_spacing, ccw) in;

layout(location = 0) in flat uint i_ids[];
layout(location = 1) in flat uint i_matids[];
layout(location = 2) in		 vec2 i_uvs[];

layout(location = 0) out flat uint o_id;
layout(location = 1) out flat uint o_matid;
layout(location = 2) out vec2 o_uv;
#ifndef OCCLUDER_PASS
layout(location = 3) out vec3 o_normal;
layout(location = 4) out vec3 o_tangent;
layout(location = 5) out vec3 o_coordw;
#endif

void main()
{
	o_id = i_ids[0];
	o_matid = i_matids[0];

	o_uv = mix(
		mix(i_uvs[0], i_uvs[1], gl_TessCoord.x),
		mix(i_uvs[3], i_uvs[2], gl_TessCoord.x),
		gl_TessCoord.y
	);

	vec3 coordw = vec3(mix(
		mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x),
		mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x),
		gl_TessCoord.y
	));
	coordw.y += texture(terrain_height_maps[o_id], o_uv).r * instance.terrains[o_id].extent.y;
	
#ifndef OCCLUDER_PASS
	o_coordw = coordw;
	o_normal = normalize(texture(terrain_normal_maps[o_id], o_uv).xyz * 2.0 - 1.0);
	o_tangent = normalize(texture(terrain_tangent_maps[o_id], o_uv).xyz * 2.0 - 1.0);
#endif
#ifndef HAS_GEOM
	#ifndef OCCLUDER_PASS
		gl_Position = camera.proj_view * vec4(coordw, 1.0);
	#else
		if (pc.i[0] == 0)
			gl_Position = lighting.dir_shadows[pc.i[1]].mats[pc.i[2]] * vec4(coordw, 1.0);
		else
			gl_Position = lighting.pt_shadows[pc.i[1]].mats[pc.i[2]] * vec4(coordw, 1.0);
	#endif
#else
	gl_Position = vec4(coordw, 1.0);
#endif
}
