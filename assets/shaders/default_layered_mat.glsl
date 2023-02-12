#ifndef DEPTH_ONLY

float tiling = material.tiling;
vec4 weights = sample_map(material.map_indices[SPLASH_MAP], i_uv * tiling);

vec4 color;
#ifdef COLOR_MAP
	color = material.color;
#else
	color = vec4(0);
#endif

float metallic;
#ifdef METALLIC_MAP
	metallic = 0;
#else
	metallic = material.metallic;
#endif

float roughness;
#ifdef ROUGHNESS_MAP
	roughness = 0;
#else
	roughness = material.roughness;
#endif

vec3 emissive;
#ifdef EMISSIVE_MAP
	emissive = vec3(0);
#else
	emissive = material.emissive.rgb;
#endif

for (int i = 0; i < LAYERS; i++)
{
	if (weights[i] > 0.0)
	{
		#ifdef TRI_PLANAR
			#ifdef COLOR_MAP
				color.rgb += textureTriPlanar(material.map_indices[COLOR_MAP + i], i_normal, i_coordw, tiling) * weights[i];
			#endif
			#ifdef METALLIC_MAP
				metallic += textureTriPlanar(material.map_indices[METALLIC_MAP + i], i_normal, i_coordw, tiling).r * weights[i];
			#endif
			#ifdef ROUGHNESS_MAP
				roughness += textureTriPlanar(material.map_indices[ROUGHNESS_MAP + i], i_normal, i_coordw, tiling).r * weights[i];
			#endif
			#ifdef EMISSIVE_MAP
				emissive.rgb += textureTriPlanar(material.map_indices[EMISSIVE_MAP + i], i_normal, i_coordw, tiling) * weights[i];
			#endif
		#else
			#ifdef COLOR_MAP
				color.rgb += sample_map(material.map_indices[COLOR_MAP + i], i_uv * tiling).rgb * weights[i];
			#endif
			#ifdef METALLIC_MAP
				metallic += sample_map(material.map_indices[METALLIC_MAP + i], i_uv * tiling).r * weights[i];
			#endif
			#ifdef ROUGHNESS_MAP
				roughness += sample_map(material.map_indices[ROUGHNESS_MAP + i], i_uv * tiling).r * weights[i];
			#endif
			#ifdef EMISSIVE_MAP
				emissive.rgb += sample_map(material.map_indices[EMISSIVE_MAP + i], i_uv * tiling).rgb * weights[i];
			#endif
		#endif
	}
}

#ifdef TINT_COLOR
	color *= material.color;
#endif
	
#ifndef GBUFFER_PASS
	vec3 albedo = (1.0 - metallic) * color.rgb;
	vec3 f0 = mix(vec3(0.04), color.rgb, metallic);
	bool receive_ssr = false;
	#ifdef RECEIVE_SSR
		receive_ssr = true;
	#endif
	o_color = vec4(shading(i_coordw, i_normal, metallic, albedo, f0, roughness, 1.0, emissive, receive_ssr), color.a);
#else
	o_gbufferA = vec4(color.rgb, 0.0);
	o_gbufferB = vec4(i_normal * 0.5 + 0.5, 0.0);
	o_gbufferC = vec4(metallic, roughness, 0.0, material.flags / 255.0);
	o_gbufferD = vec4(emissive, 0.0);
#endif

#endif // DEPTH_ONLY
