#include "mesh.pll"

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;
//layout (location = 3) in vec3 i_tangent;
//layout (location = 4) in vec3 i_bitangent;
#ifdef ARMATURE
layout (location = 5) in ivec4 i_bone_ids;
layout (location = 6) in vec4 i_bone_weights;
#endif

layout (location = 0) out flat uint o_mat_id;
layout (location = 1) out vec2 o_uv;
layout (location = 2) out vec3 o_coordw;
layout (location = 3) out vec3 o_coordv;
#ifndef SHADOW_PASS
layout (location = 4) out vec3 o_normal;
#endif

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
	o_coordw = vec3(deform * vec4(i_position, 1.0));

	#ifndef SHADOW_PASS
	o_normal = mat3(deform) * i_normal;
	#endif
#else
	o_coordw = vec3(mesh_matrices[mod_idx].transform * vec4(i_position, 1.0));

	#ifndef SHADOW_PASS
	o_normal = mat3(mesh_matrices[mod_idx].normal_matrix) * i_normal;
	#endif
#endif
	o_coordv = render_data.camera_coord - o_coordw;

	#ifndef SHADOW_PASS
	gl_Position = render_data.proj_view * vec4(o_coordw, 1.0);
	#else
	gl_Position = shadow_matrices[pc.i[0]] * vec4(o_coordw, 1.0);
	#endif
}
