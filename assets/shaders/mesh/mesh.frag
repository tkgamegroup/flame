#ifndef DEFERRED
#include "forward.pll"
#include "../shading.glsl"
#else
#include "gbuffer.pll"
#endif

layout(location = 0) in flat uint i_mat;
layout(location = 1) in vec2 i_uv;
#ifndef SHADOW_PASS
layout(location = 2) in vec3 i_normal;
#ifndef DEFERRED
layout(location = 3) in vec3 i_coordw;
layout(location = 4) in vec3 i_coordv;
#endif
#endif

#ifndef SHADOW_PASS
#ifndef DEFERRED
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;
#endif
#endif

void main()
{
#ifdef MAT
	MaterialInfo material = material_infos[i_mat];
	
#ifndef SHADOW_PASS
	vec3 N = i_normal;
#ifndef DEFERRED
	vec3 V = i_coordv;
#endif
#endif

	MAT_FILE
#else
	#ifndef SHADOW_PASS
		#if defined(PICKUP)
			o_color = pack_uint_to_v4(pc.i[0]);
		#else
			o_color = pc.f;
		#endif
	#endif
#endif
}
