layout(push_constant) uniform PushconstantT
{
	vec2 screen_size;
}pc;

layout(location = 0) out vec4 out_olor;

void main()
{
	float x = gl_FragCoord.x - pc.screen_size.x * 0.5;
	float y = gl_FragCoord.y - pc.screen_size.y * 0.5;
	out_olor = 1000.0 / (x * x + y * y) * vec4(93.0 / 255.0, 107.0 / 255.0, 105.0 / 255.0, 1);
}
