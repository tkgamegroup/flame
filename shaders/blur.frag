out vec4 o_color;

sampler2D image;

void main()
{
	ivec2 c = ivec2(gl_FragCoord.xy);
	vec3 res = texelFetch(image, c, 0).rgb * 0.3831775;
#ifdef H
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.0654205;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.2429906;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.2429906;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.0654205;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.0654205;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.2429906;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.2429906;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.0654205;
#endif
	o_color = vec4(res, 1.0);
}
