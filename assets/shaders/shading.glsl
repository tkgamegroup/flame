#include "math.glsl"

const float esm_c = 3.0;

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

vec3 lighting(vec3 N, vec3 V, vec3 L, vec3 radiance, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	vec3 H = normalize(V + L);
	
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	
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

vec4 shading(vec3 coordw, vec3 N, float metallic, vec3 albedo, vec3 spec, float roughness)
{
	vec3 ret = vec3(0.0);

	vec3 coordv = render_data.camera_coord - coordw;
	vec3 V = normalize(coordv);
	float distv = dot(render_data.camera_dir, -coordv);

	uint dir_num = tile_lights[0].dir_count;
	for (int i = 0; i < dir_num; i++)
	{
		LightInfo light = light_infos[tile_lights[0].dir_indices[i]];
		vec3 L = light.pos;
		
		float shadowed = 1.0;
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
				shadowed = clamp(exp(-esm_c * (coordl.z - ref) * shadow.far), 0.0, 1.0);
				break;
			}
		}
		
		ret += lighting(N, V, L, light.color * shadowed, metallic, albedo, spec, roughness);
	}
	
	uint pt_num = tile_lights[0].pt_count;
	for (int i = 0; i < pt_num; i++)
	{
		LightInfo light = light_infos[tile_lights[0].pt_indices[i]];
		vec3 L = light.pos - coordw;
		float dist = length(L);
		L = L / dist;

		float shadowed = 1.0;
		if (light.shadow_index != -1)
		{
			PtShadow shadow = pt_shadows[light.shadow_index];

			if (dist < shadow.far)
			{
				float ref = texture(pt_shadow_maps[light.shadow_index], -L).r * 2.0 - 1.0;
				ref = linear_depth(shadow.near, shadow.far, ref);
				shadowed = clamp(exp(-esm_c * (dist - ref)), 0.0, 1.0);
			}
		}

		ret += lighting(N, V, L, light.color / max(dist * dist, 1.0) * shadowed , metallic, albedo, spec, roughness);
	}
	
	float sky_intensity = render_data.sky_intensity;
	
	float NdotV = max(dot(N, V), 0.0);
	vec3 F = fresnel_schlick_roughness(NdotV, spec, roughness);
	vec3 kD = vec3(1.0) - F;
	kD *= 1.0 - metallic;

	// IBL
	{

		vec3 diffuse = texture(sky_irr, N).rgb * albedo;

		vec2 envBRDF = texture(sky_lut, vec2(NdotV, roughness)).rg;
		vec3 specular = textureLod(sky_rad, reflect(-V, N), roughness * render_data.sky_rad_levels).rgb * (F * envBRDF.x + envBRDF.y);

		float ao = 1.0; // TODO
		ret += (kD * diffuse + specular) * sky_intensity * ao;
	}

	ret = mix(ret, render_data.fog_color * sky_intensity, distv / render_data.zFar);

	return vec4(ret, 1.0 - kD);
}
