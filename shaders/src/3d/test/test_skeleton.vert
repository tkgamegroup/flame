layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inBoneID;
layout(location = 4) in vec4 inBoneWeight;

//layout(location = 0) out vec2 outTexcoord;
layout(location = 0) out vec3 outNormal;

layout(binding = 0) uniform ubo_matrix_
{
	mat4 proj;
	mat4 view;
	mat4 model;
}ubo_matrix;

layout(binding = 2) uniform ubo_bonematrix_
{
	mat4 v[256];
}ubo_bone;

void main()
{
	mat4 skinMatrix = inBoneWeight[0] * ubo_bone.v[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * ubo_bone.v[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * ubo_bone.v[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * ubo_bone.v[int(inBoneID[3])];
	mat4 modelMatrix = ubo_matrix.model * skinMatrix;

	//outTexcoord = inTexcoord;
	mat3 normalMatrix = transpose(inverse(mat3(ubo_matrix.view * modelMatrix)));
	outNormal = normalize(normalMatrix * inNormal);
	gl_Position = ubo_matrix.proj * ubo_matrix.view * modelMatrix * vec4(inVertex, 1);
}
