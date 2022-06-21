#include "../math.glsl"

#ifndef DEFERRED
#include "../forward.pll"
#ifdef MAT_FILE
#include "../shading.glsl"
#endif
#else
#include "../gbuffer.pll"
#endif

layout(location = 0) in flat uint i_matid;
layout(location = 1) in	     vec2 i_uv;
#ifndef OCCLUDER_PASS
layout(location = 2) in      vec3 i_normal;
layout(location = 3) in      vec3 i_coordw;
#endif

#ifndef OCCLUDER_PASS
#ifndef DEFERRED
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;
#endif
#endif

void main()
{
#ifdef MAT_FILE
	MaterialInfo material = material_infos[i_matid];
	#include MAT_FILE
#else
	#ifndef OCCLUDER_PASS
		#ifndef DEFERRED
			#ifdef PICKUP
				o_color = pack_uint_to_v4(pc.i[0]);
			#elifdef NORMAL_DATA
				o_color = vec4(i_normal * 0.5 + vec3(0.5), 1.0);
			#else
				o_color = pc.f;
			#endif
		#else
			o_res_col_met = vec4(1.0, 1.0, 1.0, 0.0);
			o_res_nor_rou = vec4(i_normal * 0.5 + 0.5, 1.0);
		#endif
	#endif
#endif
}
