#ifndef DEPTH_PASS

#ifdef TRI_PLANAR
vec3 blend = abs(i_normal);
blend = normalize(max(blend, 0.00001));
blend /= blend.x + blend.y + blend.z;
#endif

float tiling = float(material.i[0]) / 10.0;

vec4 color = vec4(0);
#if LAYERS == 1
	vec4 weights = texture(terrain_splash_maps[i_id], i_uv);
	if (weights[0] > 0.0)
		color.rgb += textureTerrain(material.map_indices[0], tiling) * weights[0];
#elif LAYERS == 2
	vec4 weights = texture(terrain_splash_maps[i_id], i_uv);
	if (weights[0] > 0.0)
		color.rgb += textureTerrain(material.map_indices[0], tiling) * weights[0];
	if (weights[1] > 0.0)
		color.rgb += textureTerrain(material.map_indices[1], tiling) * weights[1];
#elif LAYERS == 3
	vec4 weights = texture(terrain_splash_maps[i_id], i_uv);
	if (weights[0] > 0.0)
		color.rgb += textureTerrain(material.map_indices[0], tiling) * weights[0];
	if (weights[1] > 0.0)
		color.rgb += textureTerrain(material.map_indices[1], tiling) * weights[1];
	if (weights[2] > 0.0)
		color.rgb += textureTerrain(material.map_indices[2], tiling) * weights[2];
#elif LAYERS == 4
	vec4 weights = texture(terrain_splash_maps[i_id], i_uv);
	if (weights[0] > 0.0)
		color.rgb += textureTerrain(material.map_indices[0], tiling) * weights[0];
	if (weights[1] > 0.0)
		color.rgb += textureTerrain(material.map_indices[1], tiling) * weights[1];
	if (weights[2] > 0.0)
		color.rgb += textureTerrain(material.map_indices[2], tiling) * weights[2];
	if (weights[3] > 0.0)
		color.rgb += textureTerrain(material.map_indices[3], tiling) * weights[3];
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

#endif // DEPTH_PASS
