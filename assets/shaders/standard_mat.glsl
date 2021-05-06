#ifdef ALPHA_TEST
	#ifdef ALPHA_MAP
		if (texture(maps[material.map_indices[1]], i_uv).r < material.alpha_test)
			discard;
		vec4 color = texture(maps[material.map_indices[0]], i_uv);
	#else
		#ifdef COLOR_MAP
			vec4 color = texture(maps[material.map_indices[0]], i_uv);
			if (color.a < material.alpha_test)
				discard;
		#else
			vec4 color = material.color;
		#endif
	#endif
#else
	#ifdef COLOR_MAP
		vec4 color = texture(maps[material.map_indices[0]], i_uv);
	#else
		vec4 color = material.color;
	#endif
#endif
		
#ifndef SHADOW_PASS
	float metallic = material.metallic;
	float roughness = material.roughness;
	
#ifndef DEFERRED
	vec3 albedo = (1.0 - metallic) * color.rgb;
	vec3 spec = mix(vec3(0.04), color.rgb, metallic);
	o_color = vec4(shading(i_coordw, length(i_coordv), N, V, metallic, albedo, spec, roughness), 1.0);
#endif

#endif
