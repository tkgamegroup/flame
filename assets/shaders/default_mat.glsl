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

#ifdef UNLIT
	o_color = color * i_col;
#else
	#ifndef DEPTH_ONLY
		float metallic = material.metallic;
		float roughness = material.roughness;
		
		#ifndef GBUFFER_PASS
			vec3 albedo = (1.0 - metallic) * color.rgb;
			vec3 f0 = mix(vec3(0.04), color.rgb, metallic);
			o_color = vec4(shading(i_coordw, i_normal, metallic, albedo, f0, roughness, 1.0), color.a);
		#else
			o_res_col_met = vec4(color.rgb, metallic);
			o_res_nor_rou = vec4(i_normal * 0.5 + 0.5, roughness);
		#endif
	#endif
#endif
