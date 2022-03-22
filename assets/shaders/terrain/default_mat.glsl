#ifndef DEPTH_PASS

vec4 color = material.color;
vec2 uv = i_uv;
#ifdef UV_TILING
	uv *= float(material.i[0]);
#endif
#ifdef COLOR_MAP
	color = texture(material_maps[material.map_indices[COLOR_MAP]], uv);
#endif
#ifdef TINT_COLOR
	color *= material.color;
#endif
float metallic = material.metallic;
float roughness = material.roughness;
	
#ifndef DEFERRED
	vec3 albedo = (1.0 - metallic) * color.rgb;
	vec3 f0 = mix(vec3(0.04), color.rgb, metallic);
	o_color = vec4(shading(i_coordw, i_normal, metallic, albedo, f0, roughness, 1.0), color.a);
#else
	o_res_col_met = vec4(color.rgb, metallic);
	o_res_nor_rou = vec4(i_normal * 0.5 + 0.5, roughness);
#endif

#endif
