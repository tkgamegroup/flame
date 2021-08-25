#ifndef DEFERRED
#include "forward.pll"
#include "../shading.glsl"
#else
#include "../math.glsl"
#include "gbuffer.pll"
#endif

layout(location = 0) in flat uint i_id;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in vec3 i_tangent;
#ifndef DEFERRED
layout(location = 4) in vec3 i_coordw;
#endif

#ifndef DEFERRED
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;
#endif

void main()
{
#ifdef MAT
	MaterialInfo material = material_infos[terrain_infos[i_id].material_id];
	
	MAT_FILE
#else
	#ifndef SHADOW_PASS
		#if defined(PICKUP)
			o_color = pack_uint_to_v4(pc.i[0]);
		#elif defined(NORMAL_DATA)
			o_color = vec4(i_normal * 0.5 + vec3(0.5), 1.0);
		#else
			o_color = pc.f;
		#endif
	#endif
#endif
}
