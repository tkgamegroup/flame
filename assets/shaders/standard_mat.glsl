#ifdef ALPHA_TEST
	#ifdef ALPHA_MAP
		if (texture(maps[material.map_indices[ALPHA_MAP]], i_uv).r < 0.5)
			discard;
		#ifndef SHADOW_PASS
			#ifdef COLOR_MAP
				vec4 color = texture(maps[material.map_indices[COLOR_MAP]], i_uv);
				#ifdef TINT_COLOR
					color *= material.color;
				#endif
			#else
				vec4 color = material.color;
			#endif
		#endif
	#else
		#ifdef COLOR_MAP
			vec4 color = texture(maps[material.map_indices[COLOR_MAP]], i_uv);
			#ifdef TINT_COLOR
				color *= material.color;
			#endif
		#else
			vec4 color = material.color;
		#endif
		if (color.a < 0.5)
			discard;
	#endif
#else
	#ifdef COLOR_MAP
		#ifndef SHADOW_PASS
			vec4 color = texture(maps[material.map_indices[COLOR_MAP]], i_uv);
			#ifdef TINT_COLOR
				color *= material.color;
			#endif
		#endif
	#else
		vec4 color = material.color;
	#endif
#endif
		
#ifndef SHADOW_PASS
	float metallic = material.metallic;
	float roughness = material.roughness;
	
	#ifndef DEFERRED
		vec3 albedo = (1.0 - metallic) * color.rgb;
		vec3 f0 = mix(vec3(0.04), color.rgb, metallic);
		o_color = vec4(shading(i_coordw, i_normal, metallic, albedo, f0, roughness), color.a);
	#else
		o_res_col_met = vec4(color.rgb, metallic);
		o_res_nor_rou = vec4(i_normal * 0.5 + vec3(0.5), roughness);
	#endif
#endif
