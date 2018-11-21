layout(binding = 0) uniform sampler2D image;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0, index = 0) out vec4 outColor;
layout(location = 0, index = 1) out vec4 outAlpha;

void main()
{
	vec4 v = texture(image, inUV);
	outColor = inColor;
	outAlpha = v * inColor.a;
}
