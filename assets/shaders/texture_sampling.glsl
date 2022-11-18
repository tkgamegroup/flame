vec3 textureVariant(int map_id, vec2 uv)
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

	vec3 cola = textureGrad(material_maps[map_id], uv + offa, duvdx, duvdy).rgb;
	vec3 colb = textureGrad(material_maps[map_id], uv + offb, duvdx, duvdy).rgb;
	return mix(cola, colb, smoothstep(0.2, 0.8, f - 0.1 * sum(cola - colb)));
}

vec3 textureTriPlanar(int map_id, vec3 normal, vec3 coordw, float tiling)
{
	vec3 ret = vec3(0.0);
#ifdef TRI_PLANAR
	vec3 blending = abs(normal);
	blending = normalize(max(blending, 0.00001));
	blending /= blending.x + blending.y + blending.z;
	if (blending.x > 0)
		ret += textureVariant(map_id, coordw.yz * tiling) * blending.x;
	if (blending.y > 0)
		ret += textureVariant(map_id, coordw.xz * tiling) * blending.y;
	if (blending.z > 0)
		ret += textureVariant(map_id, coordw.xy * tiling) * blending.z;
#endif
	return ret;
}
