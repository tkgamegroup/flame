#ifndef DEPTH_PASS

vec2 uv = i_uv;
#ifdef UV_TILING
	uv *= float(material.i[0]);
#endif

#ifdef BLEND_BY_NORMAL
vec4 color = vec4(0);
float ndoty = dot(i_normal, vec3(0, 1, 0));
float transition = material.f[3];
flaot off = 0;
#ifdef COLOR_MAP0
	off += material.f[0];
	if (ndoty <= off - transition * 0.5)
		color += texture(material_maps[material.map_indices[COLOR_MAP0]], uv);
	else if (ndoty <= off + transition * 0.5)
		color += texture(material_maps[material.map_indices[COLOR_MAP0]], uv) * (1.0 - (off + transition * 0.5 - ndoty) / transition);
#endif
#ifdef COLOR_MAP1
	off += material.f[1];
	if (ndoty <= off - transition * 0.5)
		color += texture(material_maps[material.map_indices[COLOR_MAP1]], uv);
	else if (ndoty <= off + transition * 0.5)
		color += texture(material_maps[material.map_indices[COLOR_MAP1]], uv) * (1.0 - (off + transition * 0.5 - ndoty) / transition);
#endif
#ifdef TINT_COLOR
	color *= material.color;
#endif

#else

#ifdef COLOR_MAP
	vec4 color = texture(material_maps[material.map_indices[COLOR_MAP]], uv);
#ifdef TINT_COLOR
	color *= material.color;
#endif
#else
	vec4 color = material.color;
#endif
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
