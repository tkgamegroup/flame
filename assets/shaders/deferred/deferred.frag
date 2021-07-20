#include "deferred.pll"
#include "../shading.glsl"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	float dep = texture(img_dep, i_uv).r;
	vec4 p = render_data.proj_inv * vec4(i_uv * 2.0 - 1.0, dep, 1.0);
	p /= p.w;
	vec4 coordw = render_data.view_inv * p;

	if (dep < 1.0)
	{
		vec4 col_met = texture(img_col_met, i_uv);
		vec4 nor_rou = texture(img_nor_rou, i_uv);

		vec3 color = col_met.rgb;
		vec3 normal = nor_rou.xyz;
		float metallic = col_met.a;
		float roughness = nor_rou.a;
		vec3 albedo = (1.0 - metallic) * color;
		vec3 spec = mix(vec3(0.04), color.rgb, metallic);

		normal = nor_rou.xyz * 2.0 - vec3(1.0);
		o_color = vec4(shading(coordw.xyz, normal, metallic, albedo, spec, roughness).rgb, 1.0);
	}
	else
		o_color = vec4(texture(sky_box, normalize(coordw.xyz)).rgb * render_data.sky_intensity, 1.0);
}
