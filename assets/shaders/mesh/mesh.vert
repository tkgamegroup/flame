layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_nor;
#ifdef ARMATURE
layout(location = 3) in ivec4 i_bids;
layout(location = 4) in vec4 i_bwgts;
#endif
//layout (location = 5) in vec3 i_tan;
//layout (location = 6) in vec3 i_bit;

layout(location = 0) out flat uint o_matid;
layout(location = 1) out vec2 o_uv;
#ifndef OCCLUDER_PASS
layout(location = 2) out vec3 o_normal; 
layout(location = 3) out vec3 o_coordw;
#endif

void main()
{
	uint id = gl_InstanceIndex >> 8;
	o_matid = gl_InstanceIndex & 0xff;
	o_uv = i_uv;

#ifdef ARMATURE
	mat4 deform = mat4(0.0);
	for (int i = 0; i < 4; i++)
	{
		int bid = i_bids[i];
		if (bid == -1)
			break;
		deform += armature_instances[id].bones[bid] * i_bwgts[i];
	}

	vec3 coordw = vec3(deform * vec4(i_pos, 1.0));
	#ifndef OCCLUDER_PASS
	o_normal = normalize(transpose(inverse(mat3(deform))) * i_nor);
	#endif
#else
	vec3 coordw = vec3(mesh_instances[id].mat * vec4(i_pos, 1.0));
	#ifndef OCCLUDER_PASS
	o_normal = normalize(mat3(mesh_instances[id].nor) * i_nor);
	#endif
#endif

#ifndef OCCLUDER_PASS
	o_coordw = coordw;
	gl_Position = scene.proj_view * vec4(o_coordw, 1.0);
#else
	if (pc.i[0] == 0)
		gl_Position = dir_shadows[pc.i[1]].mats[pc.i[2]] * vec4(coordw, 1.0);
	else
		gl_Position = pt_shadows[pc.i[1]].mats[pc.i[2]] * vec4(coordw, 1.0);
#endif
}
