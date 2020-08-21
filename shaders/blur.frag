out vec4 o_color;

sampler2D image;

pushconstant
{
	vec4 kernel;
}pc;

void main()
{
	ivec2 c = ivec2(gl_FragCoord.xy);
	vec3 res = texelFetch(image, c, 0).rgb * pc.kernel[0];
#ifdef H
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * pc.kernel[3];
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * pc.kernel[2];
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * pc.kernel[1];
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * pc.kernel[1];
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * pc.kernel[2];
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * pc.kernel[3];
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * pc.kernel[3];
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * pc.kernel[2];
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * pc.kernel[1];
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * pc.kernel[1];
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * pc.kernel[2];
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * pc.kernel[3];
#endif
	o_color = vec4(res, 1.0);
}
