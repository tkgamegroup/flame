#ifndef OCCLUDER_PASS
layout(location = 0) in flat uint i_id;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in vec3 i_color;
layout(location = 4) in vec3 i_coordw;
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
	vec3 albedo;
#ifdef DONT_USE_TEXTURE
	albedo = i_color;
#else
	vec4 color = texture(material_maps[terrain_instances[i_id].grass_texture_id], i_uv);
	if (color.a < 0.5)
		discard;
	albedo = color.rgb;
#endif
#ifndef OCCLUDER_PASS
	#ifndef GBUFFER_PASS
		o_color = vec4(shading(i_coordw, i_normal, 0.0, albedo, vec3(0.04), 0.8/*roughness*/, 1.0), color.a);
	#else
		o_res_col_met = vec4(albedo, 0.0);
		o_res_nor_rou = vec4(i_normal * 0.5 + 0.5, 1.0);
	#endif
#endif
}
