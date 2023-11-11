#include "../math.glsl"
#include "../texture_sampling.glsl"
#ifndef GBUFFER_PASS
#ifdef MAT_CODE
#include "../shading.glsl"
#endif
#endif

layout(location = 0) in flat uint i_mat_id;
layout(location = 1) in	     vec2 i_uv;
#ifdef DEPTH_ONLY
layout(location = 2) in		 vec4 i_position;
#else
layout(location = 2) in flat uint i_color;
layout(location = 3) in      vec3 i_normal;
layout(location = 4) in      vec3 i_tangent;
layout(location = 5) in      vec3 i_coordw; 
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
#else
	layout(location = 0) out float o_exp_depth;
#endif

#ifdef MAT_CODE
// MATERIAL CODE BEGIN:

#include MAT_CODE

// MATERIAL CODE END:
#endif

void main()
{
#ifdef MAT_CODE
	#ifndef DEPTH_ONLY
		vec4 color = vec4(((i_color & 0x000000ff)) / 255.0, ((i_color & 0x0000ff00) >> 8) / 255.0,
			((i_color & 0x00ff0000) >> 16) / 255.0, ((i_color & 0xff000000) >> 24) / 255.0);
	#else
		vec4 color = vec4(1.0);
	#endif

	MaterialInfo material = material.infos[i_mat_id];
	material_main(material, color);
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
	#else
		#include <esm_value.glsl>
	#endif
#endif
}
