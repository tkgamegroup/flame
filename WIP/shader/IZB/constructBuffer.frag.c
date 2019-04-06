layout(binding = 0) uniform MATRIX
{
	mat4 matrixProj;
	mat4 matrixProjInv;
	mat4 matrixView;
	mat4 matrixViewInv;
	mat4 matrixProjView;
	mat4 matrixProjViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;

layout(binding = 1) uniform sampler2D depthSampler;

struct ListItem
{
	vec2 checkPosition;
	ivec2 samplePosition;
	float depth;
	uint next;
};

layout(binding = 2) buffer BUFFER
{
	mat4 shadowMatrix;
	uint count;
	ListItem item[1];
}s_buffer[32];

layout(binding = 3, r32ui) uniform coherent uimage2D i_startOffset[32];

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inViewDir;
layout(location = 2) flat in uint inIndex;

void main()
{
	/*
	float inDepth = texture(depthSampler, inTexcoord).r * 2.0 - 1.0;
	float linerDepth = LinearDepthPerspective(inDepth);
	if (linerDepth > 999.0)
		discard;
	vec3 viewDir = normalize(inViewDir);
	vec3 coordView = viewDir * (-linerDepth / viewDir.z);
	vec4 coordLight = s_buffer[inIndex].shadowMatrix * u_matrix.matrixViewInv * vec4(coordView, 1);
	coordLight /= coordLight.w;
	uint index = atomicAdd(s_buffer[inIndex].count, 1);
	uint old_head = imageAtomicExchange(i_startOffset[inIndex], ivec2((coordLight.xy * 0.5 + 0.5) * vec2(4096.0, 4096.0)), index).r;
	s_buffer[inIndex].item[index].checkPosition = coordLight.xy;
	s_buffer[inIndex].item[index].samplePosition = ivec2(gl_FragCoord.xy);
	s_buffer[inIndex].item[index].depth = coordLight.z * 0.5 + 0.5;
	s_buffer[inIndex].item[index].next = old_head;
	*/
	discard;
}
