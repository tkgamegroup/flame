const float PI = 3.14159265359;

vec3 lighting(vec3 N, vec3 V, vec3 L, vec3 intensity, vec3 albedo, vec3 spec, float roughness)
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
	return ((vec3(1.0) - F) * albedo / PI + brdf) * NdotL * intensity;
}

vec3 shading(vec3 coordw, vec3 coordv, vec3 N, vec3 V, vec3 albedo, vec3 spec, float roughness)
{
	vec3 color = vec3(0.0);

	uint light_count;
	float distancev = length(coordv);
	
	light_count = light_indices_list[0].directional_lights_count;
	for (int i = 0; i < light_count; i++)
	{
		DirectionalLightInfo light = directional_light_infos[i];
		
		vec3 L = light.dir;
		
		float shadow = 1.0;

		
		if (light.shadow_map_index != -1 && distancev < render_data.shadow_distance)
		{
			float d = distancev / render_data.shadow_distance;
			uint lvs = min(render_data.csm_levels, 4);
			float div = 1.0 / lvs;
			uint lv = 0;
			for (; lv < lvs; lv++)
			{
				float v = (lv + 1) * div;
				if (d < v * v)
					break;
			}
			lv = min(lv, lvs - 1);
			vec4 coordl = light.shadow_matrices[lv] * vec4(coordw, 1.0);
			coordl.xy = coordl.xy * 0.5 + vec2(0.5);
			if (coordl.z >= 0.0 && coordl.z <= 1.0)
			{
				float ref = texture(directional_light_shadow_maps[light.shadow_map_index], vec3(coordl.xy, lv)).r;
				shadow = clamp(exp(-100.0 * (coordl.z - ref)), 0.0, 1.0);
			}
		}
		
		vec3 intensity = light.color;
		color += lighting(N, V, L, intensity * shadow, albedo, spec, roughness);
	}
	
	light_count = light_indices_list[0].point_lights_count;
	for (int i = 0; i < light_count; i++)
	{
		PointLightInfo light = point_light_infos[light_indices_list[0].point_light_indices[i]];

		vec3 L = light.coord - coordw;
		float dist = length(L);
		L = L / dist;

		float shadow = 1.0;
		if (light.shadow_map_index != -1)
		{
			float ref = texture(point_light_shadow_maps[light.shadow_map_index], -L).r;
			shadow = clamp(exp(-0.1 * (dist - ref * light.distance)), 0.0, 1.0);
		}

		vec3 intensity = light.color / max(dist * dist * 0.01, 1.0);
		color += lighting(N, V, L, intensity * shadow, albedo, spec, roughness);
	}

	return color;
}
