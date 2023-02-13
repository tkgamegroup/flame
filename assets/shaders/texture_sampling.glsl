vec4 textureVariant(int map_id, vec2 uv)
{
#ifdef RAND_TEX_ID
	float k = sample_map(RAND_TEX_ID, 0.005 * uv).r;
#else
	float k = 0.0;
#endif

	vec2 duvdx = dFdx(uv);
	vec2 duvdy = dFdy(uv);

	float l = k * 8.0;
	float f = fract(l);

	float ia = floor(l);
	float ib = ia + 1.0;

	vec2 offa = sin(vec2(3.0, 7.0) * ia);
	vec2 offb = sin(vec2(3.0, 7.0) * ib);

	vec4 cola = textureGrad(material_maps[map_id], uv + offa, duvdx, duvdy);
	vec4 colb = textureGrad(material_maps[map_id], uv + offb, duvdx, duvdy);
	return mix(cola, colb, smoothstep(0.2, 0.8, f - 0.1 * sum(cola.rgb - colb.rgb)));
}

vec4 textureTriPlanar(int map_id, vec3 normal, vec3 world_pos, float tiling)
{
	vec4 ret = vec4(0.0);
	vec3 blending = abs(normal);
	blending = normalize(max(blending, 0.00001));
	blending /= blending.x + blending.y + blending.z;
	if (blending.x > 0)
	{
		vec2 uv = world_pos.yz * tiling;
		vec4 col;
		#ifdef TEXTURE_VARIANT
			col = textureVariant(map_id, uv);
		#else
			col = sample_map(map_id, uv);
		#endif
		ret += col * blending.x;
	}
	if (blending.y > 0)
	{
		vec2 uv = world_pos.xz * tiling;
		vec4 col;
		#ifdef TEXTURE_VARIANT
			col = textureVariant(map_id, uv);
		#else
			col = sample_map(map_id, uv);
		#endif
		ret += col * blending.y;
	}
	if (blending.z > 0)
	{
		vec2 uv = world_pos.xy * tiling;
		vec4 col;
		#ifdef TEXTURE_VARIANT
			col = textureVariant(map_id, uv);
		#else
			col = sample_map(map_id, uv);
		#endif
		ret += col * blending.z;
	}
	return ret;
}
