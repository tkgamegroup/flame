#ifndef OCCLUDER_PASS
layout(location = 0) in vec3 i_normal;
layout(location = 1) in vec3 i_color;
layout(location = 2) in vec3 i_coordw;
#endif

#include "../math.glsl"

#ifndef GBUFFER_PASS
#include "../shading.glsl"
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
#ifndef OCCLUDER_PASS
	#ifndef GBUFFER_PASS
		o_color = vec4(shading(i_coordw, i_normal, 0.0/*metallic*/, i_color/*albedo*/, vec3(0.04), 1.0/*roughness*/, 1.0), 1.0);
	#else
		o_res_col_met = vec4(i_color, 0.0);
		o_res_nor_rou = vec4(i_normal * 0.5 + 0.5, 1.0);
	#endif
#endif
}
