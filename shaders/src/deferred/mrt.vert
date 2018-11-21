#include "../ubo_matrix.glsl"
#include "object.glsl"

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
#if defined(ANIM)
layout(location = 4) in vec4 inBoneWeight;
layout(location = 5) in vec4 inBoneID;
#endif

layout(location = 0) out flat uint outMaterialID;
layout(location = 1) out vec2 outTexcoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;

void main()
{
	uint objID = gl_InstanceIndex >> 8;
	outMaterialID = gl_InstanceIndex & 0xff;
	outTexcoord = inTexcoord;
	mat4 modelMatrix = ubo_object.matrix[objID];
#if defined(ANIM)
	mat4 skinMatrix = inBoneWeight[0] * ubo_bone[objID].matrix[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * ubo_bone[objID].matrix[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * ubo_bone[objID].matrix[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * ubo_bone[objID].matrix[int(inBoneID[3])];
	modelMatrix = modelMatrix * skinMatrix;
#endif
	mat3 normalMatrix = transpose(inverse(mat3(ubo_matrix.view * modelMatrix)));
	outNormal = normalize(normalMatrix * inNormal);
	outTangent = normalize(normalMatrix * inTangent);
	gl_Position = ubo_matrix.projView * modelMatrix * vec4(inVertex, 1);
}