#ifndef TRI_PLANAR
	#define SAMPLE_MAP(ID) sample_map(material.map_indices[ID], i_uv)
#else
	float tiling = material.tiling;
	#define SAMPLE_MAP(ID) textureTriPlanar(material.map_indices[ID], i_normal, i_coordw, tiling)
#endif

#ifdef ALPHA_TEST_V
	#ifdef ALPHA_MAP
		float alpha = SAMPLE_MAP(ALPHA_MAP).r;
		#ifdef TINT_COLOR
			alpha *= material.color.a;
		#endif
		if (alpha < material.f[ALPHA_TEST_V])
			discard;
	#endif
#endif

#ifndef DEPTH_ONLY
	#ifndef HAS_COLOR
	vec4 color = vec4(1.0);
	#endif
	#ifdef COLOR_MAP
		color *= SAMPLE_MAP(COLOR_MAP);
		#ifdef TINT_COLOR
			color *= material.color;
		#endif
		#ifndef ALPHA_MAP
			#ifdef ALPHA_TEST_V
				if (color.a < material.f[ALPHA_TEST_V])
					discard;
			#endif
		#else
			color.a *= alpha;
		#endif
	#else
		color *= material.color;
	#endif

	#ifndef UNLIT
		float metallic;
		#ifdef METALLIC_MAP
			metallic = SAMPLE_MAP(METALLIC_MAP).r;
		#else
			metallic = material.metallic;
		#endif
	  
		float roughness;
		#ifdef ROUGHNESS_MAP
			roughness = SAMPLE_MAP(ROUGHNESS_MAP).r;
		#else
			roughness = material.roughness;
		#endif

		vec3 N;
		#ifdef NORMAL_MAP
			vec3 sampled_normal = normalize(SAMPLE_MAP(NORMAL_MAP).xyz * 2.0 - 1.0);
			vec3 bitangent = normalize(cross(i_tangent, i_normal));
			N = normalize(mat3(i_tangent, bitangent, i_normal) * sampled_normal * vec3(1.0, 1.0, material.normal_map_strength));
		#else
			N = i_normal;
		#endif
	#endif

	vec3 emissive;
	#ifdef EMISSIVE_MAP
		emissive = SAMPLE_MAP(EMISSIVE_MAP).rgb;
		emissive *= material.emissive_map_strength;
	#else
		emissive = material.emissive.rgb;
	#endif
		
	#ifndef GBUFFER_PASS
		#ifdef UNLIT
			o_color = color * i_col;
		#else
			vec3 albedo = (1.0 - metallic) * color.rgb;
			vec3 f0 = mix(vec3(0.04), color.rgb, metallic);
			bool receive_ssr = false;
			#ifdef RECEIVE_SSR
				receive_ssr = true;
			#endif
			o_color = vec4(shading(i_coordw, N, metallic, albedo, f0, roughness, 1.0, emissive, receive_ssr), color.a);
		#endif
	#else
		o_gbufferA = vec4(color.rgb, 0.0);
		o_gbufferB = vec4(N * 0.5 + 0.5, 0.0);
		o_gbufferC = vec4(metallic, roughness, 0.0, material.flags / 255.0);
		o_gbufferD = vec4(emissive, 0.0);
	#endif
#endif
