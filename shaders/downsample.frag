out vec4 o_color;

sampler2D image;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy) * 2;
	o_color = vec4(texelFetch(image, coord, 0).rgb * 0.25 +
		texelFetch(image, coord + ivec2(1, 0), 0).rgb * 0.25 +
		texelFetch(image, coord + ivec2(0, 1), 0).rgb * 0.25 +
		texelFetch(image, coord + ivec2(1, 1), 0).rgb * 0.25, 1.0);
}
