out vec4 o_color;

sampler2D image;

void main()
{
	ivec2 c = ivec2(gl_FragCoord.xy);

#ifdef R1
	vec3 res = texelFetch(image, c, 0).rgb * 0.904419;
#ifdef H
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.04779;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.04779;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.04779;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.04779;
#endif
#endif

#ifdef R2
	vec3 res = texelFetch(image, c, 0).rgb * 0.52495;
#ifdef H
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.015885;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.015885;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.015885;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.015885;
#endif
#endif

#ifdef R3
	vec3 res = texelFetch(image, c, 0).rgb * 0.382925;
#ifdef H
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.005977;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.005977;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.005977;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.005977;
#endif
#endif

	o_color = vec4(res, 1.0);
}
