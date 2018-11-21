struct Particle
{
	vec4 coord;
};

layout(binding = 0) uniform ubo_
{
	ivec4 data;
	Particle particles[256];
}ubo;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main()
{
	float v = 0.0;
	vec2 c = inTexcoord * vec2(ubo.data.y, ubo.data.z);
	for (int i = 0; i < ubo.data.x; i++)
	{
		float len = max(1.0, length(c - vec2(ubo.particles[i].coord)));
		len *= len;
		v += 500.0 / len;
	}
		
	if (v >= 0.5)
		outColor = vec4(1.0);
	else if (v >= 0.47)
		outColor = vec4(1.0, 1.0, 1.0, (v - 0.47) / 0.03);
	else
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
}
