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

vec3 shading(vec3 coordw, vec3 coordv, vec3 N, vec3 V, float metallic, vec3 albedo, vec3 spec, float roughness)
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
				if (d <= v * v)
					break;
			}
			vec4 coordl = light.shadow_matrices[lv] * vec4(coordw, 1.0);
			coordl.xy = coordl.xy * 0.5 + vec2(0.5);
			if (coordl.z >= 0.0 && coordl.z <= 1.0)
			{
				float ref = texture(directional_shadow_maps[light.shadow_map_index], vec3(coordl.xy, lv)).r;
				shadow = clamp(exp(-esm_c * render_data.zFar * (coordl.z - ref)), 0.0, 1.0);
			}
		}
		
		color += lighting(N, V, L, light.color * shadow, metallic, albedo, spec, roughness);
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
			float ref = texture(point_shadow_maps[light.shadow_map_index], -L).r;
			shadow = clamp(exp(-esm_c * (dist - ref * light.distance)), 0.0, 1.0);
		}

		vec3 intensity = light.color / max(dist * dist * 0.01, 1.0);
		color += lighting(N, V, L, intensity * shadow, metallic, albedo, spec, roughness);
	}

	return color;
}
