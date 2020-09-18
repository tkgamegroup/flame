#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#include "material_dsl.glsl"
#include "light_dsl.glsl"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coordw;
layout (location = 3) in vec3 i_coordv;
layout (location = 4) in vec3 i_normal;

layout (location = 0) out vec4 o_color;

const float PI = 3.14159265359;

vec3 shading(vec3 N, vec3 V, vec3 L, vec3 intensity, vec3 albedo, vec3 spec, float roughness)
{
	vec3 H = normalize(V + L);
	
	float NdotV = clamp(dot(N, V), 0.0, 1.0);
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
	float NdotH = clamp(dot(N, H), 0.0, 1.0);
	float LdotH = clamp(dot(L, H), 0.0, 1.0);
	
	float roughness2 = roughness * roughness;
	float roughness4 = roughness2 * roughness2;

	float d = NdotH * NdotH * (roughness4 - 1.0) + 1.0;
	float D = roughness4 / (PI * d * d);

	float LdotH5 = 1.0 - LdotH;
	LdotH5 = LdotH5 * LdotH5 * LdotH5 * LdotH5 * LdotH5;
	vec3 F = spec + (1.0 - spec) * LdotH5;

	float k = roughness2 / 2.0;
	float G = (1.0 / (NdotL * (1.0 - k) + k)) * (1.0 / (NdotV * (1.0 - k) + k));

	vec3 brdf = min(F * G * D, vec3(1.0));
	return ((vec3(1.0) - F) * albedo + brdf) * NdotL * intensity;
}

float compute_shadow(float d, float z)
{
	float ret = exp(-0.2 * (d - z));
	return ret >= 1.0 ? 1.0 : 0.3 * ret;
}

void main()
{
	vec3 res = vec3(0.0);

	MaterialInfo material = material_infos[i_mat_id];

	vec4 color;
	if (material.color_map_index >= 0)
		color = texture(maps[material.color_map_index], i_uv);
	else
		color = material.color;

	if (color.a < material.alpha_test)
		discard;

	float metallic;
	float roughness;
	if (material.metallic_roughness_ao_map_index >= 0)
	{
		vec4 s = texture(maps[material.metallic_roughness_ao_map_index], i_uv);
		metallic = s.r;
		roughness = s.g;
	}
	else
	{
		metallic = material.metallic;
		roughness = material.roughness;
	}
	vec3 albedo = (1.0 - metallic) * color.rgb;
	vec3 spec = mix(vec3(0.04), color.rgb, metallic);

	vec3 N = normalize(i_normal);
	vec3 V = normalize(i_coordv);
	
	uint light_count;
	
	light_count = light_indices_list[0].directional_lights_count;
	for (int i = 0; i < light_count; i++)
	{
		DirectionalLightInfo light = directional_light_infos[i];
		
		vec3 L = light.dir;
		
		float shadow = 1.0;

		if (light.shadow_map_index != -1)
		{
			vec4 coord = light.shadow_matrices[0] * vec4(i_coordw, 1.0);
			float ref = texture(directional_light_shadow_maps[light.shadow_map_index], vec3(coord.xy * 0.5 + vec2(0.5), 0)).r;
			shadow = compute_shadow(coord.z * light.shadow_distances[0], ref);
		}
		
		vec3 intensity = light.color;
		res += shading(N, V, L, intensity * shadow, albedo, spec, roughness);
	}
	
	light_count = light_indices_list[0].point_lights_count;
	for (int i = 0; i < light_count; i++)
	{
		PointLightInfo light = point_light_infos[light_indices_list[0].point_light_indices[i]];

		vec3 L = light.coord - i_coordw;
		float dist = length(L);
		L = L / dist;

		float shadow = 1.0;
		if (light.shadow_map_index != -1)
		{
			float ref = texture(point_light_shadow_maps[light.shadow_map_index], -L).r;
			shadow = compute_shadow(dist, ref);
		}

		vec3 intensity = light.color / max(dist * dist * 0.01, 1.0);
		res += shading(N, V, L, intensity * shadow, albedo, spec, roughness);
	}
	
	o_color = vec4(res, 1.0);
}
