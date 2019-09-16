void main()
{
	float x = gl_FragCoord.x - pc.screen_size.x * 0.5;
	float y = gl_FragCoord.y - pc.screen_size.y * 0.5;
	out_color = vec4(1000.0 / (x * x + y * y) * vec3(93.0 / 255.0, 107.0 / 255.0, 105.0 / 255.0), 1);
}
