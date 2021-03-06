#ifndef DEFERRED
#include "forward.pll"
#else
#include "gbuffer.pll"
#endif

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_nor;
#ifdef ARMATURE
layout(location = 3) in ivec4 i_bids;
layout(location = 4) in vec4 i_bwgts;
#endif
//layout (location = 5) in vec3 i_tan;
//layout (location = 6) in vec3 i_bit;

layout(location = 0) out flat uint o_mat;
layout(location = 1) out vec2 o_uv;
#ifndef SHADOW_PASS
layout(location = 2) out vec3 o_normal; 
#ifndef DEFERRED
layout(location = 3) out vec3 o_coordw;
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
		int bid = i_bids[i];
		if (bid == -1)
			break;
		deform += armatures[idx].bones[bid] * i_bwgts[i];
	}

	vec3 coordw = vec3(deform * vec4(i_pos, 1.0));

	#ifndef SHADOW_PASS
		o_normal = mat3(deform) * i_nor;
	#endif
#else
	vec3 coordw = vec3(transforms[idx].mat * vec4(i_pos, 1.0));

	#ifndef SHADOW_PASS
		o_normal = mat3(transforms[idx].nor) * i_nor;
	#endif
#endif

#if !defined(DEFERRED) && !defined(SHADOW_PASS)
	o_coordw = coordw;
#endif

#ifndef SHADOW_PASS
	gl_Position = render_data.proj_view * vec4(coordw, 1.0);
#else
	switch (pc.i[0])
	{
	case 0:
		gl_Position = dir_shadows[pc.i[1]].mats[pc.i[2]] * vec4(coordw, 1.0);
		break;
	case 1:
		gl_Position = pt_shadows[pc.i[1]].mats[pc.i[2]] * vec4(coordw, 1.0);
		break;
	}
#endif
}
