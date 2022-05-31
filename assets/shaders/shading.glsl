float distribution_GGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness * roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return min(4.0, num / denom);
}

float geometry_schlick_GGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = geometry_schlick_GGX(NdotV, roughness);
	float ggx1  = geometry_schlick_GGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cos_theta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}

vec3 fresnel_schlick_roughness(float cos_theta, vec3 f0, float roughness)
{
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cos_theta, 5.0);
}   

vec3 brdf(vec3 N, vec3 V, vec3 L, vec3 radiance, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	vec3 H = normalize(V + L);
	
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = dot(N, L);
	if (NdotL <= 0.0)
		return vec3(0.0);
	
	float NDF = distribution_GGX(N, H, roughness);        
	float G   = geometry_smith(N, V, L, roughness);      
	vec3  F   = fresnel_schlick(max(dot(H, V), 0.0), f0);
	
	vec3 kD = vec3(1.0) - F;
	kD *= 1.0 - metallic;	  
		
	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * NdotV * NdotL;
	vec3 specular     = numerator / max(denominator, 0.001);
				   
	return (kD * albedo / PI + specular) * radiance * NdotL;
}

const float esm_c = 3.0;

vec3 get_lighting(vec3 coordw, float distv, vec3 N, vec3 V, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	vec3 ret = vec3(0.0);

	uint dir_num =  light_grids[0].count;
	for (int i = 0; i < dir_num; i++)
	{
		LightInfo light = light_infos[light_indexs[i]];
		vec3 L = light.pos;
		
		float f_shadow = 1.0;
		if (light.shadow_index != -1)
		{
			DirShadow shadow = dir_shadows[light.shadow_index];

			for (uint lv = 0; lv < 4; lv++)
			{
				float split = shadow.splits[lv];
				if (distv >= split)
					continue;
				vec4 coordl = shadow.mats[lv] * vec4(coordw, 1.0);
				coordl.xy = coordl.xy * 0.5 + vec2(0.5);
				float ref = texture(dir_shadow_maps[light.shadow_index], vec3(coordl.xy, lv)).r;
				f_shadow = clamp(exp(-esm_c * (coordl.z - ref) * shadow.far), 0.0, 1.0);
				break;
			}
		}
		
		if (f_shadow > 0.0)
		{
			vec3 radiance = light.color * f_shadow;
			ret += brdf(N, V, L, radiance, metallic, albedo, f0, roughness);
			#ifdef DOUBLE_SIDE
				ret += brdf(-N, V, L, radiance, metallic, albedo, f0, roughness);
			#endif
		}
	}
	
	LightGrid grid = light_grids[1];
	uint idx_off = grid.offset;
	uint pt_num = grid.count;
	pt_num = 0;
	for (int i = 0; i < pt_num; i++)
	{
		LightInfo light = light_infos[light_indexs[idx_off + i]];
		vec3 L = light.pos - coordw;
		float dist = length(L);
		L = L / dist;

		float f_shadow = 1.0;
		if (light.shadow_index != -1)
		{
			PtShadow shadow = pt_shadows[light.shadow_index];

			if (dist < shadow.far)
			{
				float ref = texture(pt_shadow_maps[light.shadow_index], -L).r * 2.0 - 1.0;
				ref = linear_depth(shadow.near, shadow.far, ref);
				f_shadow = clamp(exp(-esm_c * (dist - ref)), 0.0, 1.0);
			}
		}
		
		if (f_shadow > 0.0)
		{
			vec3 radiance = light.color / max(dist * dist, 1.0) * f_shadow;
			ret += brdf(N, V, L, radiance, metallic, albedo, f0, roughness);
			#ifdef DOUBLE_SIDE
				ret += brdf(-N, V, L, radiance, metallic, albedo, f0, roughness);
			#endif
		}
	}

	return ret;
}

vec3 get_ibl(vec3 N, vec3 V, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	vec3 F = fresnel_schlick_roughness(NdotV, f0, roughness);
	vec3 kD = vec3(1.0) - F;
	kD *= 1.0 - metallic;

	vec3 diffuse = texture(sky_irr_map, cube_coord(N)).rgb * albedo;

	vec2 envBRDF = texture(brdf_map, vec2(NdotV, roughness)).rg;
	vec3 specular = textureLod(sky_rad_map, cube_coord(reflect(-V, N)), roughness * scene.sky_rad_levels).rgb * (F * envBRDF.x + envBRDF.y);

	return (kD * diffuse + specular) * scene.sky_intensity;
}

vec3 get_fog(vec3 color, float dist)
{
	return mix(color, scene.fog_color * scene.sky_intensity, dist / scene.zFar);
}

vec3 shading(vec3 coordw, vec3 N, float metallic, vec3 albedo, vec3 f0, float roughness, float ao)
{
	vec3 ret = vec3(0.0);

	vec3 V = scene.camera_coord - coordw;
	float distv = dot(scene.camera_dir, -V);
	V = normalize(V);

#ifndef CAMERA_LIGHT
	ret += get_lighting(coordw, distv, N, V, metallic, albedo, f0, roughness);
	ret += get_ibl(N, V, metallic, albedo, f0, roughness) * ao;
	ret = get_fog(ret, distv);
#else
	ret += brdf(N, V, -scene.camera_dir, vec3(3.14), metallic, albedo, f0, roughness);
#endif
	
	return ret;
}
