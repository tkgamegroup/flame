#include "mesh.pll"
#include "../shading.glsl"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coordw;
layout (location = 3) in vec3 i_coordv;
#ifndef SHADOW_PASS
layout (location = 4) in vec3 i_normal;
#endif

#ifndef SHADOW_PASS
layout (location = 0) out vec4 o_color;
#else
layout (location = 0) out float o_depth;
#endif

void main()
{
	#ifdef MAT
		MaterialInfo material = material_infos[i_mat_id];
		
		#ifndef SHADOW_PASS
		vec3 N = normalize(i_normal);
		vec3 V = normalize(i_coordv);
		#endif
	
		MAT_FILE

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
