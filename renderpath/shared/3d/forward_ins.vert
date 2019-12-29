layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outWorldCoord;

layout(set = 1, binding = 0) uniform U_matrix
{
	mat4 proj;
	mat4 view;
}u_matrix;

layout(set = 1, binding = 1) uniform U_matrix_ins
{
	mat4 model[1024];
}u_matrix_ins;

void main()
{
	outTexcoord = inTexcoord;
	mat4 model = u_matrix_ins.model[gl_InstanceIndex];
	outWorldCoord = vec3(model * vec4(inVertex, 1));
	mat3 normalMatrix = transpose(inverse(mat3(u_matrix.view * model)));
	outNormal = normalize(normalMatrix * inNormal);
	gl_Position = u_matrix.proj * u_matrix.view * vec4(outWorldCoord, 1);
}
