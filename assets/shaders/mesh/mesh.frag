#ifndef DEFERRED
#include "forward.pll"
#include "../shading.glsl"
#else
#include "deferred.pll"
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
#else
layout(location = 0) out float o_depth;
#endif

void main()
{
#ifdef MAT
	MaterialInfo material = material_infos[i_mat];

	vec3 N = i_normal;
#ifndef DEFERRED
	vec3 V = i_coordv;
#endif

	MAT_FILE

#ifdef DEFERRED
	o_res_col_met = vec4(color.rgb, metallic);
	o_res_nor_rou = vec4(N * 0.5 + vec3(0.5), roughness);
#endif

#ifdef SHADOW_PASS
	if (pc.f[0] == 0.0)
		o_depth = gl_FragCoord.z;
	else
		o_depth = pc.f[0] / (pc.f[1] + gl_FragCoord.z * (pc.f[0] - pc.f[1]));
#endif

#else

#ifdef PICKUP
	o_color = pack_uint_to_v4(pc.i[0]);
#else
	o_color = pc.f;
#endif

#endif
}
