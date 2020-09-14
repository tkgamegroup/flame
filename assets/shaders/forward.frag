#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec3 i_coordw;
layout (location = 2) in vec3 i_coordv;
layout (location = 3) in vec3 i_normal;
layout (location = 4) in vec2 i_uv;

struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	float alpha_test;
	float dummy0;
	int color_map_index;
	int metallic_roughness_ao_map_index;
	int normal_height_map_index;
	int dummy1;
};

layout (set = 0, binding = 1) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[];
};

layout (set = 0, binding = 2) uniform sampler2D maps[128];

struct LightInfo
{
	int type;
	int cast_shadow;
	int dummy0;
	int dummy1;

	vec3 color;
	int dummy2;
	vec3 pos;
	int dummy3;

	uint shadow_map_indices[8]; // only 6 valid, 8 is for alignment
	mat4 shadow_map_matrices[6];
};

layout (set = 1, binding = 1) buffer readonly LightInfos
{
	LightInfo light_infos[];
};

struct LightIndices
{
	uint count;
	uint indices[1023];
};

layout (set = 1, binding = 2) buffer readonly LightIndicesList
{
	LightIndices light_indices_list[];
};

layout (set = 1, binding = 3) uniform sampler2D shadow_maps[24];

layout (location = 0) out vec4 o_color;

const float PI = 3.14159265359;

int cube_side(vec3 r) 
{
   vec3 absr = abs(r);
   if (absr.x > absr.y && absr.x > absr.z) 
	 return int(step(r.x, 0.0));
   else if (absr.y > absr.z) 
     return int(step(r.y, 0.0)) + 2;
   else
     return int(step(r.z, 0.0)) + 4;
}

void main()
{
	o_color = vec4(0);

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
	float NdotV = clamp(dot(N, V), 0.0, 1.0);

	uint count = light_indices_list[0].count;
	for (int i = 0; i < count; i++)
	{
		LightInfo light = light_infos[light_indices_list[0].indices[i]];

		if (light.cast_shadow == 1)
		{
			if (light.type == 1)
			{
				int idx = cube_side(i_coordw - light.pos);
				vec4 coord = light.shadow_map_matrices[idx] * vec4(i_coordw, 1.0);
				coord /= coord.w;
				float ref = texture(shadow_maps[light.shadow_map_indices[idx]], coord.xy * 0.5 + vec2(0.5)).r;
				if (ref < coord.z - 0.005)
					continue;
			}
		}

		vec3 L;
		vec3 Li;
		if (light.type == 0)
		{
			L = normalize(light.pos);
			Li = light.color;
		}
		else
		{
			L = light.pos - i_coordw;
			float dist = length(L);
			L = L / dist;
			Li = light.color / max(dist * dist * 0.01, 1.0);
		}

		vec3 H = normalize(V + L);
		
		float NdotL = clamp(dot(N, L), 0.0, 1.0);
		float NdotH = clamp(dot(N, H), 0.0, 1.0);
		float LdotH = clamp(dot(L, H), 0.0, 1.0);

		float roughness2 = roughness * roughness;

		float roughness4 = roughness2 * roughness2;
		float denom = NdotH * NdotH *(roughness4 - 1.0) + 1.0;
		float D = roughness4 / (PI * denom * denom);

		float LdotH5 = 1.0 - LdotH;
		LdotH5 = LdotH5*LdotH5*LdotH5*LdotH5*LdotH5;
		vec3 F = spec + (1.0 - spec) * LdotH5;

		float k = roughness2 / 2.0;
		float G = (1.0 / (NdotL * (1.0 - k) + k)) * (1.0 / (NdotV * (1.0 - k) + k));

        vec3 specular = min(F * G * D, vec3(1.0));

		o_color.rgb += ((vec3(1.0) - F) * albedo + specular) * NdotL * Li;
	}

	o_color.a = color.a;
}
