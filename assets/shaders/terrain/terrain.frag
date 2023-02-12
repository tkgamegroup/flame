#include "../math.glsl"
#include "../texture_sampling.glsl"
#ifndef GBUFFER_PASS
#ifdef MAT_CODE
#include "../shading.glsl"
#endif
#endif

layout(location = 0) in vec2 i_uv;
#ifndef DEPTH_ONLY
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec3 i_tangent;
layout(location = 3) in vec3 i_coordw;
#endif

#ifndef DEPTH_ONLY
#ifndef GBUFFER_PASS
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_gbufferA;
layout(location = 1) out vec4 o_gbufferB;
layout(location = 2) out vec4 o_gbufferC;
layout(location = 3) out vec4 o_gbufferD;
#endif

#endif

void main()
{
#ifdef MAT_CODE
	MaterialInfo material = material.infos[pc.index & 0xffff];
	#include MAT_CODE
#else
	#ifndef DEPTH_ONLY
		#ifndef GBUFFER_PASS
			#ifdef PICKUP
				o_color = pack_uint_to_v4(pc.i[0]);
			#elifdef NORMAL_DATA
				o_color = vec4(i_normal * 0.5 + 0.5, 1.0);
			#else
				o_color = pc.f;
			#endif
		#else
			o_gbufferA = vec4(1.0, 1.0, 1.0, 0.0);
			o_gbufferB = vec4(i_normal * 0.5 + 0.5, 0.0);
			o_gbufferC = vec4(0.0, 1.0, 0.0, 0.0);
			o_gbufferD = vec4(0.0, 0.0, 0.0, 0.0);
		#endif
	#endif
#endif
}
