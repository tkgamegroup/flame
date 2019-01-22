layout(binding = 0) uniform sampler2D i_src;

layout(location = 0) in vec2 i_uv;

layout(location = 0) out vec4 o_color;

void main()
{
	o_color = texture(i_src, i_uv);
}
