// PBR Reference:
// https://www.shadertoy.com/view/ld3SRr

float distribution_term(vec3 N, vec3 H, float roughness)
{
	float NdotH = dot(N, H);
	float alpha2 = roughness * roughness;
	float NoH2 = NdotH * NdotH;
	float den = NoH2 * (alpha2 - 1.0) + 1.0;
	return (NdotH > 0.) ? alpha2 / (PI * den * den) : 0.0;
}

float geometry_term(float NdotV, float NdotL, float roughness)
{
	float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
	float gv = max(NdotV, 0.0) / (NdotV * (1.0 - k) + k);
	float gl = max(NdotL, 0.0) / (NdotL * (1.0 - k) + k);
	return gv * gl;
}

vec3 fresnel_term(float cos_theta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}   

vec3 fresnel_term_roughness(float cos_theta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cos_theta, 5.0);
}   

vec3 brdf(vec3 N, vec3 V, vec3 L, vec3 light_color, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	float NdotL = dot(N, L);
	if (NdotL <= 0.0)
		return vec3(0.0);

	vec3 H = normalize(V + L);
	float NdotV = dot(N, V);
	vec3 radiance = light_color * NdotL;
	
	float D = distribution_term(N, H, roughness);
	vec3  F = fresnel_term(dot(V, H), f0);
	float G = geometry_term(NdotV, NdotL, roughness);
		
	vec3 diffuse = albedo * radiance / PI * (1.0 - F) * (1.0 - metallic);
	vec3 specular = D * F * G / (4.0 * NdotV * NdotL + 0.0001) * radiance;
				   
	return diffuse + specular;
}

vec3 get_lighting(vec3 world_pos, float distv, vec3 N, vec3 V, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	vec3 ret = vec3(0.0);

	uint dir_num = lighting.dir_lights_count;
	for (int i = 0; i < dir_num; i++)
	{
		DirLight li = lighting.dir_lights[lighting.dir_lights_list[i]];
		vec3 L = li.dir;
		
		float f_shadow = 1.0;
		if (li.shadow_index != -1)
		{
			for (uint lv = 0; lv < 4; lv++)
			{
				vec4 splits = lighting.dir_shadows[li.shadow_index].splits;
				if (distv < splits[lv])
				{
					vec4 coordl = lighting.dir_shadows[li.shadow_index].mats[lv] * vec4(world_pos, 1.0);
					coordl.xy = coordl.xy * 0.5 + 0.5;
					float ref = texture(dir_shadow_maps[li.shadow_index], vec3(coordl.xy, lv)).r;
					f_shadow *= clamp(exp(-lighting.esm_factor * (coordl.z - ref)), 0.0, 1.0);
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
	
	uint pt_num = lighting.pt_lights_count;
	for (int i = 0; i < pt_num; i++)
	{
		PtLight li = lighting.pt_lights[lighting.pt_lights_list[i]];
		vec3 L = li.pos - world_pos;
		float dist = length(L);
		L = L / dist;

		float f_shadow = 1.0;
		if (li.shadow_index != -1)
		{
			float far = lighting.pt_shadows[li.shadow_index].far;
			if (dist < far)
			{
				float ref = texture(pt_shadow_maps[li.shadow_index], -L).r * 2.0 - 1.0;
				ref = linear_depth(lighting.pt_shadows[li.shadow_index].near, far, ref);
				f_shadow = clamp(exp(-lighting.esm_factor * (dist - ref)), 0.0, 1.0);
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

vec3 get_env(vec3 N, vec3 V, vec3 world_pos, float metallic, vec3 albedo, vec3 f0, float roughness, bool receive_ssr)
{
	float NdotV = max(dot(N, V), 0.0);
	vec3 F = fresnel_term_roughness(NdotV, f0, roughness);
	vec2 envBRDF = texture(brdf_map, vec2(NdotV, 1.0 - roughness)).rg;

	if (receive_ssr && lighting.ssr_enable == 1)
	{
		mat4 proj_mat = camera.proj;
		mat4 view_mat = camera.last_view;
		mat3 view_mat3 = mat3(view_mat);
		float zNear = camera.zNear;
		float zFar = camera.zFar;
		vec3 view_N = view_mat3 * N;
		vec3 view_V = view_mat3 * V;
		vec3 view_R = reflect(-view_V, view_N);
		vec3 view_pos = (view_mat * vec4(world_pos, 1.0)).xyz;
		vec3 hit_pos = view_pos;

		float thickness = lighting.ssr_thickness;
		float ray_step = lighting.ssr_step;
		int max_steps = lighting.ssr_max_steps;
		int num_binary_search_steps = lighting.ssr_binary_search_steps;

		vec3 dir = view_R * ray_step;
		for (int i = 0; i < max_steps; i++)
		{
			hit_pos += dir;
			vec4 p = proj_mat * vec4(hit_pos, 1.0);
			p /= p.w;
			if (p.x < -1 || p.x > +1 || p.y < -1 || p.y > +1) 
				break;

			p.xy = p.xy * 0.5 + 0.5;
			float d = texture(img_last_dep, p.xy).r;
			d = linear_depth(zNear, zFar, d * 2.0 - 1.0);
			p.z = linear_depth(zNear, zFar, p.z * 2.0 - 1.0);
			if (d < p.z && d > p.z - thickness)
			{
				hit_pos -= dir * 0.5;
				for (int j = 0; j < num_binary_search_steps; j++)
				{
					p = proj_mat * vec4(hit_pos, 1.0);
					p /= p.w;
					p.xy = p.xy * 0.5 + 0.5;
					d = texture(img_last_dep, p.xy).r;
					d = linear_depth(zNear, zFar, d * 2.0 - 1.0);
					dir *= 0.5;
					if (d < p.z)
						hit_pos -= dir;
					else
						hit_pos += dir;
				}
				return texture(img_last_dst, p.xy).rgb * (F * envBRDF.x + envBRDF.y);
			}
		}
	}

	vec3 diffuse = texture(sky_irr_map, cube_coord(N)).rgb * albedo * (1.0 - F) * (1.0 - metallic);
	vec3 specular = textureLod(sky_rad_map, cube_coord(reflect(-V, N)), roughness * lighting.sky_rad_levels).rgb * (F * envBRDF.x + envBRDF.y);

	return (diffuse + specular) * lighting.sky_intensity * 1.0; // TODO: replace 1.0 to ao when SSAO is ready
}

vec3 get_fog(vec3 color, float dist)
{
	return mix(color, lighting.fog_color * lighting.sky_intensity, smoothstep(0.0, camera.zFar, dist));
}

vec3 shading(vec3 world_pos, vec3 N, float metallic, vec3 albedo, vec3 f0, float roughness, float ao, bool receive_ssr)
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

	vec3 V = camera.coord - world_pos;
	float distv = dot(camera.front, -V);
	V = normalize(V);

	ret += get_lighting(world_pos, distv, N, V, metallic, albedo, f0, roughness);
	ret += get_env(N, V, world_pos, metallic, albedo, f0, roughness, receive_ssr);
	ret = get_fog(ret, distv);
#ifdef POST_SHADING_CODE
	#include POST_SHADING_CODE
#endif
	
	return ret;
}
