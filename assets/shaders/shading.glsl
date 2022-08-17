float distribution_term(vec3 N, vec3 H, float roughnessL)
{
	float NdotH  = max(dot(N, H), 0.0);
	float r2 = roughnessL * roughnessL;
	float d = (NdotH * r2 - NdotH) * NdotH + 1.0;
	return r2 / (d * d * PI + 1e-7f);
}

float geometry_term(vec3 N, vec3 V, vec3 L, float roughnessL)
{
	float r2 = roughnessL * roughnessL;
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float gv = NdotL * sqrt(NdotV * (NdotV - NdotV * r2) + r2);
	float gl = NdotV * sqrt(NdotL * (NdotL - NdotL * r2) + r2);
	return 0.5 / max(gv + gl, 0.00001);
}

vec3 fresnel_term(float cos_theta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}   

vec3 brdf(vec3 N, vec3 V, vec3 L, vec3 light_color, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	float NdotL = dot(N, L);
	if (NdotL <= 0.0)
		return vec3(0.0);

	vec3 H = normalize(V + L);
	float NdotV = max(dot(N, V), 0.0);
	vec3 radiance = light_color * NdotL;
	
	float roughnessL = max(0.01, roughness * roughness);
	float D = distribution_term(N, H, roughnessL);        
	float G = geometry_term(N, V, L, roughnessL);      
	vec3  F = fresnel_term(max(dot(H, V), 0.0), f0); 
		
	vec3 diffuse = albedo * radiance;
	vec3 specular = D * G * F * PI * radiance;
				   
	return diffuse + specular;
}

const float esm_c = 7.0;

vec3 get_lighting(vec3 coordw, float distv, vec3 N, vec3 V, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	vec3 ret = vec3(0.0);

	uint dir_num = light_grids[0].count;
	for (int i = 0; i < dir_num; i++)
	{
		LightInfo li = light_infos[light_indexs[i]];
		vec3 L = li.pos;
		
		float f_shadow = 1.0;
		if (li.shadow_index != -1)
		{
			for (uint lv = 0; lv < 4; lv++)
			{
				if (distv < dir_shadows[li.shadow_index].splits[lv])
				{
					vec4 coordl = dir_shadows[li.shadow_index].mats[lv] * vec4(coordw, 1.0);
					coordl.xy = coordl.xy * 0.5 + vec2(0.5);
					float ref = texture(dir_shadow_maps[li.shadow_index], vec3(coordl.xy, lv)).r;
					f_shadow *= clamp(exp(-esm_c * (coordl.z - ref) * dir_shadows[li.shadow_index].far), 0.0, 1.0);
					break;
				}
			}
		}
		
		if (f_shadow > 0.0)
		{
			vec3 light_color = li.color * f_shadow;
			ret += brdf(N, V, L, light_color, metallic, albedo, f0, roughness);
			#ifdef DOUBLE_SIDE
				ret += brdf(-N, V, L, light_color, metallic, albedo, f0, roughness);
			#endif
		}
	}
	
	uint idx_off = light_grids[1].offset;
	uint pt_num = light_grids[1].count;
	pt_num = 0;
	for (int i = 0; i < pt_num; i++)
	{
		LightInfo li = light_infos[light_indexs[idx_off + i]];
		vec3 L = li.pos - coordw;
		float dist = length(L);
		L = L / dist;

		float f_shadow = 1.0;
		if (li.shadow_index != -1)
		{
			if (dist < pt_shadows[li.shadow_index].far)
			{
				float ref = texture(pt_shadow_maps[li.shadow_index], -L).r * 2.0 - 1.0;
				ref = linear_depth(pt_shadows[li.shadow_index].near, pt_shadows[li.shadow_index].far, ref);
				f_shadow = clamp(exp(-esm_c * (dist - ref)), 0.0, 1.0);
			}
		}
		
		if (f_shadow > 0.0)
		{
			vec3 light_color = li.color / max(dist * dist, 1.0) * f_shadow;
			ret += brdf(N, V, L, light_color, metallic, albedo, f0, roughness);
			#ifdef DOUBLE_SIDE
				ret += brdf(-N, V, L, light_color, metallic, albedo, f0, roughness);
			#endif
		}
	}

	return ret;
}

vec3 get_env(vec3 N, vec3 V, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	vec3 diffuse = texture(sky_irr_map, cube_coord(N)).rgb / PI * albedo * (1.0 - metallic);

	vec2 envBRDF = texture(brdf_map, vec2(NdotV, 1.0 - roughness)).rg;
	vec3 specular = textureLod(sky_rad_map, cube_coord(reflect(-V, N)), roughness * lighting.sky_rad_levels).rgb * (f0 * envBRDF.x + envBRDF.y);

	return (diffuse + specular) * lighting.sky_intensity;
}

vec3 get_fog(vec3 color, float dist)
{
	return mix(color, lighting.fog_color * lighting.sky_intensity, smoothstep(0.0, camera.zFar, dist));
}

vec3 shading(vec3 coordw, vec3 N, float metallic, vec3 albedo, vec3 f0, float roughness, float ao)
{
#ifdef ALBEDO_DATA
	return albedo;
#endif
#ifdef NORMAL_DATA
	return N;
#endif
#ifdef METALLIC_DATA
	return vec3(metallic);
#endif
#ifdef ROUGHNESS_DATA
	return vec3(roughness);
#endif

	vec3 ret = vec3(0.0);

	vec3 V = camera.coord - coordw;
	float distv = dot(camera.front, -V);
	V = normalize(V);

	ret += get_lighting(coordw, distv, N, V, metallic, albedo, f0, roughness);
	ret += get_env(N, V, metallic, albedo, f0, roughness) * /*ao*/1.0; // TODO: use ao when ssao is ok
	ret = get_fog(ret, distv);
	
	return ret;
}
