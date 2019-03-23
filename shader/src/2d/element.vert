layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform uPushConstant
{
	vec2 scale;
	vec2 sdf_range;
}pc;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out flat uint outID;

void main()
{
	outColor = aColor;
	outUV = aUV;
	outID = gl_InstanceIndex;
	gl_Position = vec4(aPos * pc.scale - vec2(1.0), 0, 1);
}
