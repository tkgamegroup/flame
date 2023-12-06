// PBR Reference:
// https://www.shadertoy.com/view/ld3SRr

#define FOG_NONE 0
#define FOG_LINEAR 1
#define FOG_EXP 2
#define FOG_EXP2 3
#define FOG_HEIGHT_LINEAR 4
#define FOG_HEIGHT_EXP 5
#define FOG_HEIGHT_EXP2 6

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

	uint csm_levels = lighting.csm_levels;
	uint dir_num = lighting.dir_lights_count;
	for (int i = 0; i < dir_num; i++)
	{
		DirLight li = lighting.dir_lights[lighting.dir_lights_list[i]];
		vec3 L = li.dir;
		
		float f_shadow = 1.0;
		if (li.shadow_index != -1)
		{
			for (uint lv = 0; lv < csm_levels; lv++)
			{
				vec4 splits = lighting.dir_shadows[li.shadow_index].splits;
				float far = splits[lv];
				if (distv < far)
				{
					vec4 coordl = lighting.dir_shadows[li.shadow_index].mats[lv] * vec4(world_pos, 1.0);
					coordl /= coordl.w;
					coordl.xy = coordl.xy * 0.5 + 0.5;
					float ref = texture(dir_shadow_maps[li.shadow_index], vec3(coordl.xy, lv)).r;
					float shadow_factor = saturate(ref * exp(-lighting.esm_factor * coordl.z));
					shadow_factor = linstep(lighting.shadow_bleeding_reduction, 1.0, shadow_factor);
					shadow_factor = lighting.shadow_darkening + (1.0 - lighting.shadow_darkening) * shadow_factor;
					f_shadow *= shadow_factor;
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
				float ref = texture(pt_shadow_maps[li.shadow_index], -L).r;
				dist /= far;
				float shadow_factor = saturate(ref * exp(-lighting.esm_factor * dist));
				shadow_factor = linstep(lighting.shadow_bleeding_reduction, 1.0, shadow_factor);
				shadow_factor = lighting.shadow_darkening + (1.0 - lighting.shadow_darkening) * shadow_factor;
				f_shadow *= shadow_factor;
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
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);
	float ssr_visibility = 0.0;

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

		float thickness = lighting.ssr_thickness;
		float max_distance = lighting.ssr_max_distance;
		int max_steps = lighting.ssr_max_steps;
		int num_binary_search_steps = lighting.ssr_binary_search_steps;
		vec2 tex_size = camera.viewport;

		vec3 start_pos = view_pos;
		vec3 end_pos = view_pos + view_R * max_distance;

		vec4 start_frag = proj_mat * vec4(start_pos, 1.0);
		start_frag /= start_frag.w;
		start_frag.xy = start_frag.xy * 0.5 + 0.5;
		start_frag.xy *= tex_size;

		vec4 end_frag = proj_mat * vec4(end_pos, 1.0);
		end_frag /= end_frag.w;
		end_frag.xy = end_frag.xy * 0.5 + 0.5;
		end_frag.xy *= tex_size;

		vec2 frag = start_frag.xy;
		vec2 uv = frag / tex_size;

		float deltaX = end_frag.x - start_frag.x;
		float deltaY = end_frag.y - start_frag.y;
		float useX = abs(deltaX) >= abs(deltaY) ? 1.0 : 0.0;
		int delta = min(max_steps, int(mix(abs(deltaY), abs(deltaX), useX) * 0.3));
		vec2 increment = vec2(deltaX, deltaY) / max(delta, 0.001);

		float search0 = 0;
		float search1 = 0;

		int hit0 = 0;
		int hit1 = 0;

		float diff = 0;

		for (int i = 0; i < delta; i++)
		{
			frag += increment;
			uv = frag / tex_size;
			float sampled_depth = texture(img_last_dep, uv).r;
			sampled_depth = linear_depth(zNear, zFar, sampled_depth * 2.0 - 1.0);

			search1 = clamp(mix((frag.y - start_frag.y) / deltaY, (frag.x - start_frag.x) / deltaX, useX), 0.0, 1.0);
			float depth = (start_pos.z * end_pos.z) / -mix(end_pos.z, start_pos.z, search1);
			diff = depth - sampled_depth;
			if (diff > 0 && diff < thickness)
			{
				hit0 = 1;
				break;
			}
			else
				search0 = search1;
		}

		/*
		search1 = search0 + ((search1 - search0) / 2.0);
		num_binary_search_steps *= hit0;

		for (int i = 0; i < num_binary_search_steps; i++)
		{
			frag = mix(start_frag.xy, end_frag.xy, search1);
			uv = frag / tex_size;
			float sampled_depth = texture(img_last_dep, uv).r;
			sampled_depth = linear_depth(zNear, zFar, sampled_depth * 2.0 - 1.0);

			float depth = (start_pos.z * end_pos.z) / -mix(end_pos.z, start_pos.z, search1);
			diff = depth - sampled_depth;

			if (diff > 0 && diff < thickness)
			{
				hit1 = 1;
				search1 = search0 + ((search1 - search0) / 2);
			}
			else 
			{
				float temp = search1;
				search1 = search1 + ((search1 - search0) / 2);
				search0 = temp;
			}
		}
		*/

		ssr_visibility = hit0
			* (uv.x < 0 || uv.x > 1 ? 0 : 1)
			* (uv.y < 0 || uv.y > 1 ? 0 : 1);

		ssr_visibility = clamp(ssr_visibility, 0, 1);

		specular += texture(img_last_dst, uv).rgb * (F * envBRDF.x + envBRDF.y);
	}

	float ao = 1.0; // replace 1.0 to ao when SSAO is ready
	float sky_intensity = lighting.sky_intensity * ao;
	diffuse += texture(sky_irr_map, cube_coord(N)).rgb * albedo * (1.0 - F) * (1.0 - metallic) * sky_intensity;
	specular = mix(textureLod(sky_rad_map, cube_coord(reflect(-V, N)), roughness * lighting.sky_rad_levels).rgb * (F * envBRDF.x + envBRDF.y) * sky_intensity, specular, ssr_visibility);

	return diffuse + specular;
}

vec3 get_fog(vec3 color, float dist, float h)
{
	float f = 1.0;
	if (lighting.fog_type == FOG_LINEAR)
		f = map_01(dist, lighting.fog_start, lighting.fog_end);
	else if (lighting.fog_type == FOG_EXP)
	{
		f = map_01(dist, lighting.fog_start, lighting.fog_end);
		f = 1.0 - exp(-f);
	}
	else if (lighting.fog_type == FOG_EXP2)
	{
		f = map_01(dist, lighting.fog_start, lighting.fog_end);
		f = 1.0 - exp(-(f * f));
	}
	else if (lighting.fog_type == FOG_HEIGHT_LINEAR)
	{
		f = map_01(dist, lighting.fog_start, lighting.fog_end);
		float fd = 1.0 - map_01(h, lighting.fog_base_height, lighting.fog_max_height);
		f = max(f, fd);
	}
	else if (lighting.fog_type == FOG_HEIGHT_EXP)
	{
		f = map_01(dist, lighting.fog_start, lighting.fog_end);
		f = 1.0 - exp(-f);
		float fd = map_01(h, lighting.fog_base_height, lighting.fog_max_height);
		fd = exp(-fd);
		f = max(f, fd);
	}
	else if (lighting.fog_type == FOG_HEIGHT_EXP2)
	{
		f = map_01(dist, lighting.fog_start, lighting.fog_end);
		f = 1.0 - exp(-(f * f));
		float fd = map_01(h, lighting.fog_base_height, lighting.fog_max_height);
		fd = exp(-(fd * fd));
		f = max(f, fd);
	}
	return mix(color, lighting.fog_color * lighting.sky_intensity, f * lighting.fog_density);
}

vec3 shading(vec3 world_pos, vec3 N, float metallic, vec3 albedo, vec3 f0, float roughness, float ao, vec3 emissive, bool receive_ssr)
{
	vec3 ret = emissive;

	vec3 V = camera.coord - world_pos;
	float distv = dot(camera.front, -V);
	V = normalize(V);

	ret += get_lighting(world_pos, distv, N, V, metallic, albedo, f0, roughness);
	ret += get_env(N, V, world_pos, metallic, albedo, f0, roughness, receive_ssr);
	ret = get_fog(ret, distv, world_pos.y);
#ifdef POST_SHADING_CODE
	#include POST_SHADING_CODE
#endif
	
	return ret;
}
