layout(location = 0) in vec3 inVertex;

layout(location = 0) out vec4 outCol;

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
	uint idx = gl_InstanceIndex + 1;
	float r = (idx % 0xff) / 255.f;
	idx /= 0xff;
	float g = (idx % 0xff) / 255.f;
	idx /= 0xff;
	float b = (idx % 0xff) / 255.f;
	idx /= 0xff;
	float a = (idx % 0xff) / 255.f;
	outCol = vec4(r, g, b, a);

	gl_Position = ubo_matrix.proj * ubo_matrix.view * 
	ubo_matrix_ins.model[gl_InstanceIndex] * vec4(inVertex, 1);
}
