#ifndef DEPTH_ONLY

vec4 color = vec4(0);
#if LAYERS == 1
	if (weights[0] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[0], i_normal, i_coordw, tiling) * weights[0];
#elif LAYERS == 2
	if (weights[0] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[0], i_normal, i_coordw, tiling) * weights[0];
	if (weights[1] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[1], i_normal, i_coordw, tiling) * weights[1];
#elif LAYERS == 3
	if (weights[0] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[0], i_normal, i_coordw, tiling) * weights[0];
	if (weights[1] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[1], i_normal, i_coordw, tiling) * weights[1];
	if (weights[2] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[2], i_normal, i_coordw, tiling) * weights[2];
#elif LAYERS == 4
	if (weights[0] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[0], i_normal, i_coordw, tiling) * weights[0];
	if (weights[1] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[1], i_normal, i_coordw, tiling) * weights[1];
	if (weights[2] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[2], i_normal, i_coordw, tiling) * weights[2];
	if (weights[3] > 0.0)
		color.rgb += textureTriPlanar(material.map_indices[3], i_normal, i_coordw, tiling) * weights[3];
#endif

#ifdef TINT_COLOR
	color *= material.color;
#endif

float metallic = material.metallic;
float roughness = material.roughness;
vec3 emissive = material.emissive.rgb;
	
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
