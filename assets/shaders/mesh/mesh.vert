#include "mesh.vi"

layout(location = 0) out flat uint o_matid;
layout(location = 1) out vec2 o_uv;
#ifndef DEPTH_ONLY
layout(location = 2) out vec3 o_normal; 
layout(location = 3) out vec3 o_tangent; 
layout(location = 4) out vec3 o_coordw;
#endif

void main()
{
	uint id = gl_InstanceIndex & 0xffff;
	o_matid = gl_InstanceIndex >> 16;
	o_uv = i_uv;

#ifdef ARMATURE
	mat4 deform = mat4(0.0);
	for (int i = 0; i < 4; i++)
	{
		int bid = i_bids[i];
		if (bid == -1)
			break;
		deform += instance.armatures[id].bones[bid] * i_bwgts[i];
	}

	vec3 world_pos = vec3(deform * vec4(i_pos, 1.0));
	#ifndef DEPTH_ONLY
		mat3 normal_mat = transpose(inverse(mat3(deform)));
		o_normal = normalize(normal_mat * i_nor);
		o_tangent = normalize(normal_mat * i_tan);
	#endif
#else
	vec3 world_pos = vec3(instance.meshes[id].mat * vec4(i_pos, 1.0));
	#ifndef DEPTH_ONLY
		mat3 normal_mat = mat3(instance.meshes[id].nor);
		o_normal = normalize(normal_mat * i_nor);
		o_tangent = normalize(normal_mat * i_tan);
	#endif
#endif

#ifdef DEPTH_ONLY
	if (pc.i[0] == 0)
		gl_Position = lighting.dir_shadows[pc.i[1]].mats[pc.i[2]] * vec4(world_pos, 1.0);
	else
		gl_Position = lighting.pt_shadows[pc.i[1]].mats[pc.i[2]] * vec4(world_pos, 1.0);
#else
	gl_Position = camera.proj_view * vec4(world_pos, 1.0);
	o_coordw = world_pos;
#endif
}
