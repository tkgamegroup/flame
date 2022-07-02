#ifdef ALPHA_TEST
	#ifdef ALPHA_MAP
		vec4 color;
		color.a = texture(material_maps[material.map_indices[ALPHA_MAP]], i_uv).r;
		if (color.a < ALPHA_TEST)
			discard;
		#ifndef OCCLUDER_PASS
			#ifdef COLOR_MAP
				color.rgb = texture(material_maps[material.map_indices[COLOR_MAP]], i_uv).rgb;
				#ifdef TINT_COLOR
					color *= material.color;
				#endif
			#else
				vec4 color = material.color;
			#endif
		#endif
	#else
		#ifdef COLOR_MAP
			vec4 color = texture(material_maps[material.map_indices[COLOR_MAP]], i_uv);
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
		#ifndef OCCLUDER_PASS
			vec4 color = texture(material_maps[material.map_indices[COLOR_MAP]], i_uv);
			#ifdef TINT_COLOR
				color *= material.color;
			#endif
		#endif
	#else
		vec4 color = material.color;
	#endif
#endif
		
#ifndef OCCLUDER_PASS
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
