layout (location = 0) in vec2 i_uv;

layout (location = 0) out vec2 o_uv;

void main()
{
	o_uv = i_uv;
	gl_Position = vec4(i_uv * 2.0 - 1.0, 1.0, 1.0);
}
