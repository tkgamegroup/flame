#include "math.glsl"

const float esm_c = 3.0;

float distribution_GGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
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
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(max(1.0 - cos_theta, 0.0), 5.0);
}   

vec3 lighting(vec3 N, vec3 V, vec3 L, vec3 radiance, float metallic, vec3 albedo, vec3 f0, float roughness)
{
	vec3 H = normalize(V + L);
	
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	
	float NDF = distribution_GGX(N, H, roughness);        
    float G   = geometry_smith(N, V, L, roughness);      
    vec3 F    = fresnel_schlick(max(dot(H, V), 0.0), f0);
	
	vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specular     = numerator / max(denominator, 0.001);
	               
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 shading(vec3 coordw, float distancev, vec3 N, vec3 V, float metallic, vec3 albedo, vec3 spec, float roughness)
{
	vec3 ret = vec3(0.0);

	uint light_count;
	
	light_count = grid_lights[0].dir_count;
	for (int i = 0; i < light_count; i++)
	{
		LightInfo light = light_infos[grid_lights[0].dir_indices[i]];
		vec3 L = light.pos;
		
		float shadow = 1.0;
		if (light.shadow_index != -1 && distancev < light.shadow_distance)
		{
			float d = distancev / light.shadow_distance;
			uint lvs = render_data.csm_levels;
			float div = 1.0 / lvs;
			uint lv = 0;
			for (; lv < lvs; lv++)
			{
				float v = (lv + 1) * div;
				if (d <= v * v)
					break;
			}
			vec4 coordl = dir_shadow_mats[light.shadow_index * 4 + lv] * vec4(coordw, 1.0);
			coordl.xy = coordl.xy * 0.5 + vec2(0.5);
			float ref = texture(dir_shadow_maps[light.shadow_index], vec3(coordl.xy, lv)).r;
			ref = ref * light.shadow_distance;
			float dist = coordl.z * light.shadow_distance;
			shadow = clamp(exp(-esm_c * (dist - ref)), 0.0, 1.0);
		}
		
		ret += lighting(N, V, L, light.color * shadow, metallic, albedo, spec, roughness);
	}

	
	light_count = grid_lights[0].pt_count;
	for (int i = 0; i < light_count; i++)
	{
		LightInfo light = light_infos[grid_lights[0].pt_indices[i]];
		vec3 L = light.pos - coordw;
		float dist = length(L);
		L = L / dist;

		float shadow = 1.0;
		if (light.shadow_index != -1 && distancev < light.shadow_distance)
		{
			float ref = texture(pt_shadow_maps[light.shadow_index], -L).r * 2.0 - 1.0;
			float zNear = render_data.ptsm_near;
			float zFar = light.shadow_distance;
			ref = 2.0 * zNear * zFar / (zFar + zNear - ref * (zFar - zNear));
			shadow = clamp(exp(-esm_c * (dist - ref)), 0.0, 1.0);
		}

		vec3 intensity = light.color / max(dist * dist * 0.01, 1.0);
		ret += lighting(N, V, L, intensity * shadow, metallic, albedo, spec, roughness);
	}

	{
		float NdotV = max(dot(N, V), 0.0);
		vec3 F = fresnel_schlick_roughness(NdotV, spec, roughness);
		vec2 envBRDF = texture(sky_lut, vec2(NdotV, roughness)).rb;
  
		float ao = 0.2; // TODO
		ret += ((1.0 - F) * (1.0 - metallic) * texture(sky_irr, N).rgb * albedo + 
			textureLod(sky_rad, reflect(-V, N), roughness * render_data.sky_rad_levels).rgb * (F * envBRDF.x + envBRDF.y)) * ao;
	}

	ret = mix(ret, vec3(1), distancev / 1000.0); // TODO: replace this basic fog

	return ret;
}
