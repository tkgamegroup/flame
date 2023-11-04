#ifndef DEPTH_ONLY

void material_main(MaterialInfo material, vec4 color)
{
	float tiling = material.tiling;
	vec4 weights = sample_map(material.map_indices[SPLASH_MAP], i_uv * tiling);

	#ifndef TRI_PLANAR
		#define SAMPLE_MAP(ID) sample_map(material.map_indices[ID], i_uv * tiling)
	#else
		#define SAMPLE_MAP(ID) textureTriPlanar(material.map_indices[ID], i_normal, i_coordw, tiling)
	#endif

	#ifdef COLOR_MAP
		color *= material.color;
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

	vec3 N;
	#ifdef NORMAL_MAP
		vec3 sampled_normal = vec3(0);
	#else
		N = i_normal;
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
			#ifdef COLOR_MAP
				color += SAMPLE_MAP(COLOR_MAP + i) * weights[i];
			#endif
			#ifdef METALLIC_MAP
				metallic += SAMPLE_MAP(METALLIC_MAP + i).r * weights[i];
			#endif
			#ifdef ROUGHNESS_MAP
				roughness += SAMPLE_MAP(ROUGHNESS_MAP + i).r * weights[i];
			#endif
			#ifdef NORMAL_MAP
				sampled_normal += SAMPLE_MAP(NORMAL_MAP + i).xyz * weights[i];
			#endif
			#ifdef EMISSIVE_MAP
				emissive += SAMPLE_MAP(EMISSIVE_MAP + i) * weights[i];
			#endif
		}
	}

	#ifdef NORMAL_MAP
		vec3 sampled_normal = sampled_normal * 2.0 - 1.0);
		vec3 bitangent = normalize(cross(i_tangent, i_normal));
		N = normalize(mat3(i_tangent, bitangent, i_normal) * sampled_normal * vec3(1.0, 1.0, material.normal_map_strength));
	#endif

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
		o_color = vec4(shading(i_coordw, N, metallic, albedo, f0, roughness, 1.0, emissive, receive_ssr), color.a);
	#else
		o_gbufferA = vec4(color.rgb, 0.0);
		o_gbufferB = vec4(N * 0.5 + 0.5, 0.0);
		o_gbufferC = vec4(metallic, roughness, 0.0, material.flags / 255.0);
		o_gbufferD = vec4(emissive, 0.0);
	#endif
}

#endif // DEPTH_ONLY