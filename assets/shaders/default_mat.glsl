#ifdef ALPHA_TEST
	#ifdef ALPHA_MAP
		vec4 color;
		color.a = sample_map(material.map_indices[ALPHA_MAP], i_uv).r;
		if (color.a < ALPHA_TEST)
			discard;
		#ifndef DEPTH_ONLY
			#ifdef COLOR_MAP
				color.rgb = sample_map(material.map_indices[COLOR_MAP], i_uv).rgb;
				#ifdef TINT_COLOR
					color *= material.color;
				#endif
			#else
				vec4 color = material.color;
			#endif
		#endif
	#else
		#ifdef COLOR_MAP
			vec4 color = sample_map(material.map_indices[COLOR_MAP], i_uv);
			#ifdef TINT_COLOR
				color *= material.color;
			#endif
		#else
			vec4 color = material.color;
		#endif
		if (color.a < ALPHA_TEST)
			discard;
	#endif
#else
	#ifdef COLOR_MAP
		#ifndef DEPTH_ONLY
			vec4 color = sample_map(material.map_indices[COLOR_MAP], i_uv);
			#ifdef TINT_COLOR
				color *= material.color;
			#endif
		#endif
	#else
		vec4 color = material.color;
	#endif
#endif

#ifndef DEPTH_ONLY

	float metallic;
	#ifdef METALLIC_MAP
		metallic = sample_map(material.map_indices[METALLIC_MAP], i_uv).r;
	#else
		metallic = material.metallic;
	#endif

	float roughness;
	#ifdef ROUGHNESS_MAP
		roughness = sample_map(material.map_indices[ROUGHNESS_MAP], i_uv).r;
	#else
		roughness = material.roughness;
	#endif

	vec3 N = i_normal;
	#ifdef NORMAL_MAP
		vec3 sampled_normal = normalize(sample_map(material.map_indices[NORMAL_MAP], i_uv).xyz * 2.0 - 1.0);
		vec3 bitangent = normalize(cross(i_tangent, i_normal));
		N = normalize(mat3(i_tangent, bitangent, i_normal) * sampled_normal * vec3(1.0, 1.0, material.normal_map_strength));
	#endif

	vec3 emissive;
	#ifdef EMISSIVE_MAP
		emissive = sample_map(material.map_indices[EMISSIVE_MAP], i_uv).rgb;
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
