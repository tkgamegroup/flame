#ifndef DEFERRED
#include "forward.pll"
#else
#include "gbuffer.pll"
#endif

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
//layout (location = 3) in vec3 i_tangent;
//layout (location = 4) in vec3 i_bitangent;
#ifdef ARMATURE
layout(location = 5) in ivec4 i_bone_ids;
layout(location = 6) in vec4 i_bone_weights;
#endif

layout(location = 0) out flat uint o_mat;
layout(location = 1) out vec2 o_uv;
#ifndef SHADOW_PASS
layout(location = 2) out vec3 o_normal; 
#ifndef DEFERRED
layout(location = 3) out vec3 o_coordw;
layout(location = 4) out vec3 o_coordv;
#endif
#endif

void main()
{
	uint idx = gl_InstanceIndex >> 16;
	o_mat = gl_InstanceIndex & 0xffff;
	o_uv = i_uv;

#ifdef ARMATURE
	mat4 deform = mat4(0.0);
	for (int i = 0; i < 4; i++)
	{
		int id = i_bone_ids[i];
		if (id != -1)
			deform += i_bone_weights[i] * armatures[idx].bones[id];
	}

	vec3 coordw = vec3(deform * vec4(i_position, 1.0));

#ifndef SHADOW_PASS
	o_normal = mat3(deform) * i_normal;
#endif

#else

	vec3 coordw = vec3(transforms[idx].mat * vec4(i_position, 1.0));

#ifndef SHADOW_PASS
	o_normal = mat3(transforms[idx].nor) * i_normal;
#endif

#endif

#if !defined(DEFERRED) && !defined(SHADOW_PASS)
	o_coordw = coordw;
	o_coordv = render_data.camera_coord - coordw;
#endif

#ifndef SHADOW_PASS
	gl_Position = render_data.proj_view * vec4(coordw, 1.0);
#else
	switch (pc.i[0])
	{
	case 0:
		gl_Position = dir_shadow_mats[pc.i[1]] * vec4(coordw, 1.0);
		break;
	case 1:
		gl_Position = pt_shadow_mats[pc.i[1]] * vec4(coordw, 1.0);
		break;
	}
#endif
}
