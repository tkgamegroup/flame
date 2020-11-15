layout (location = 0) in vec3 i_position;
layout (location = 1) in vec4 i_color;

layout (location = 0) out vec4 o_color;

layout (push_constant) uniform PushConstantT
{
	mat4 proj_view;
}pc;

void main()
{
	o_color = i_color;
	gl_Position = pc.proj_view * vec4(i_position, 1);
}
