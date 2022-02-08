#include "deferred.pll"
#include "../shading.glsl"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec4 o_color;

void main()
{
	float dep = texture(img_dep, i_uv).r;
	vec4 p = scene.proj_inv * vec4(i_uv * 2.0 - 1.0, dep, 1.0);
	p /= p.w;
	vec4 coordw = scene.view_inv * p;

	if (dep < 1.0)
	{
		vec4 col_met = texture(img_col_met, i_uv);
		vec4 nor_rou = texture(img_nor_rou, i_uv);
		float ao = texture(img_ao, i_uv).r;

		vec3 color = col_met.rgb;
		vec3 normal = nor_rou.xyz;
		float metallic = col_met.a;
		float roughness = nor_rou.a;
		vec3 albedo = (1.0 - metallic) * color;
		vec3 spec = mix(vec3(0.04), color.rgb, metallic);

		normal = normalize(nor_rou.xyz * 2.0 - 1.0);
		o_color = vec4(shading(coordw.xyz, normal, metallic, albedo, spec, roughness, ao), 1.0);
	}
	else
		o_color = vec4(texture(sky_box, cube_coord(normalize(coordw.xyz))).rgb * scene.sky_intensity, 1.0);
}
