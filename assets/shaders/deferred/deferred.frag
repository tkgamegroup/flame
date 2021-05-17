#include "deferred.pll"
#include "../shading.glsl"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	float dep = texture(img_dep, i_uv).r;
	vec4 coordv = render_data.proj_inv * vec4(i_uv * 2.0 - 1.0, dep, 1.0);
	coordv /= coordv.w;
	vec4 coordw = render_data.view_inv * coordv;

	if (dep < 1.0)
	{
		vec4 col_met = texture(img_col_met, i_uv);
		vec4 nor_rou = texture(img_nor_rou, i_uv);
		vec4 coordw = render_data.view_inv * coordv;

		vec3 color = col_met.rgb;
		vec3 normal = nor_rou.xyz;
		float metallic = col_met.a;
		float roughness = nor_rou.a;
		vec3 albedo = (1.0 - metallic) * color;
		vec3 spec = mix(vec3(0.04), color.rgb, metallic);
#ifdef NORMAL_DATA
		o_color = vec4(normal, 1.0);
#else
		normal = nor_rou.xyz * 2.0 - vec3(1.0);
		o_color = vec4(shading(coordw.xyz, length(coordv.xyz), normal, normalize(coordv.xyz), metallic, albedo, spec, roughness), 1.0);
#endif
	}
	else
		o_color = texture(sky_box, normalize(coordw.xyz));
}
