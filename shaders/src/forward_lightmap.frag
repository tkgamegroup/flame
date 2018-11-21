layout(set = 0, binding = 1) uniform sampler2D i_col;

layout(location = 0) in vec2 i_uv;

layout(location = 0) out vec4 fColor;

void main()
{
	fColor = texture(i_col, i_uv);
	//fColor = vec4(1.0);
}
