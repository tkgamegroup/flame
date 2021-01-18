#include "depth.pll"

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec2 i_uv;
#ifdef ARMATURE
layout (location = 2) in ivec4 i_bone_ids;
layout (location = 3) in vec4 i_bone_weights;
#endif

layout (location = 0) out flat uint o_mat_id;
layout (location = 1) out vec2 o_uv;

void main()
{
	uint mod_idx = gl_InstanceIndex >> 16;
	o_mat_id = gl_InstanceIndex & 0xffff;
	o_uv = i_uv;
#ifdef ARMATURE
	mat4 deform = mat4(0.0);
	for (int i = 0; i < 4; i++)
	{
		int id = i_bone_ids[i];
		if (id != -1)
			deform += i_bone_weights[i] * bones[id];
	}
	gl_Position = pc.proj_view * deform * vec4(i_position, 1.0);
#else
	gl_Position = pc.proj_view * mesh_matrices[mod_idx].transform * vec4(i_position, 1.0);
#endif
}
