#include "../math.glsl"
#include "../texture_sampling.glsl"
#ifndef GBUFFER_PASS
#ifdef MAT_CODE
#include "../shading.glsl"
#endif
#endif

#ifndef OCCLUDER_PASS
layout(location = 0) in vec3 i_normal;
layout(location = 1) in vec3 i_coordw;
#endif

#ifndef OCCLUDER_PASS
#ifndef GBUFFER_PASS
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;
#endif
#endif

void main()
{
#ifdef MAT_CODE
	MaterialInfo material = material.infos[pc.index & 0xffff];
	float tiling = float(material.f[0]);
	
	#ifndef OCCLUDER_PASS
		float radians = asin(i_normal.y) / 3.14159;
		vec3 w;
		w[0] = mix(1, 0, radians + 0.5);
		w[1] = mix(0, 1, radians + 0.5);
		w[2] = 0;
		w /= w[0] + w[1] + w[2];
		vec4 weights = vec4(w, 0);
	#else
		vec4 weights = vec4(0);
	#endif

	#include MAT_CODE
#else
	#ifndef OCCLUDER_PASS
		#ifndef GBUFFER_PASS
			#ifdef PICKUP
				o_color = pack_uint_to_v4(pc.i[0]);
			#elifdef NORMAL_DATA
				o_color = vec4(i_normal * 0.5 + 0.5, 1.0);
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
