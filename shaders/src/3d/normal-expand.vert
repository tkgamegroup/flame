layout(location = 0) in vec3 inVertex;
layout(location = 2) in vec3 inNormal;

layout(set = 1, binding = 0) uniform ubo_matrix_
{
	mat4 proj;
	mat4 view;
}ubo_matrix;

layout(set = 1, binding = 1) uniform ubo_matrix_ins_
{
	mat4 model[1024];
}ubo_matrix_ins;

void main()
{
	mat4 modelview = ubo_matrix.view * ubo_matrix_ins.model[gl_InstanceIndex & 0xffff];
	gl_Position = ubo_matrix.proj * modelview * vec4(inVertex + inNormal * 0.02, 1);
}
