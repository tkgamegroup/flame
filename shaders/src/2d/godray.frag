layout(location = 0) in vec2 inTexcoord;

layout(binding = 0) uniform ubo_godrayparm_
{
	vec2 pos;
	float intensity;
}ubo_godrayparm;

layout(binding = 1) uniform sampler2D img_source;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 sum = vec3(0.0);
	vec2 tc = inTexcoord;
	vec2 step = (ubo_godrayparm.pos - tc) / 32.0;
	for (int i = 0; i < 32; i++)
	{
		sum += pow(texture(img_source, tc).rgb, vec3(50.0));
		tc += step;
	}
	outColor = vec4(sum / 32 * ubo_godrayparm.intensity, 1);
}
