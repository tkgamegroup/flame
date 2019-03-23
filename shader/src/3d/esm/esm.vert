#if defined(ANIM)
layout(binding = 2) uniform ubo_object_animated_
{
	mat4 matrix[8];
}ubo_object;
#else
layout(binding = 2) uniform ubo_object_static_
{
	mat4 matrix[1024];
}ubo_object;
#endif

layout(binding = 14) uniform ubo_shadow_
{
	mat4 matrix[8];
}ubo_shadow;

#if defined(ANIM)
layout(set = 2, binding = 0) uniform ubo_bone_
{
	mat4 matrix[256];
}ubo_bone[8];
#endif

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
#if defined(ANIM)
layout(location = 4) in vec4 inBoneWeight;
layout(location = 5) in vec4 inBoneID;
#endif

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out flat uint outMaterialID;

void main()
{
	uint objID;
	uint shadowID;
	{
		uint v = gl_InstanceIndex >> 8;
		objID = v & 0x80000;
		shadowID = v >> 20;
		outMaterialID = gl_InstanceIndex & 0xff;
	}
	outTexcoord = inTexcoord;
	mat4 modelMatrix = ubo_object.matrix[objID];
#if defined(ANIM)
	mat4 skinMatrix = inBoneWeight[0] * ubo_bone[objID].matrix[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * ubo_bone[objID].matrix[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * ubo_bone[objID].matrix[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * ubo_bone[objID].matrix[int(inBoneID[3])];
	modelMatrix = modelMatrix * skinMatrix;
#endif
	gl_Position = ubo_shadow.matrix[shadowID] * modelMatrix * vec4(inVertex, 1);
}