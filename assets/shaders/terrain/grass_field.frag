#ifndef DEPTH_ONLY
layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec3 i_color;
layout(location = 3) in vec3 i_coordw;
#endif

#include "../math.glsl"

#ifndef GBUFFER_PASS
#include "../shading.glsl"
#endif

layout(location = 0) out vec4 o_color;

void main()
{
	vec3 albedo;
#ifdef DONT_USE_TEXTURE
	albedo = i_color;
#else
	vec4 color = sample_map(instance.terrains[pc.index >> 16].grass_texture_id, i_uv);
	if (color.a < 0.5)
		discard;
	albedo = color.rgb;
#endif
	o_color = vec4(shading(i_coordw, i_normal, 0.0, albedo, vec3(0.04), 0.8/*roughness*/, 1.0, vec3(0), false), color.a);
}
