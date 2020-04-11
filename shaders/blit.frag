in vec2 i_coord;

out vec4 o_color;

sampler2D image;

void main()
{
	o_color = texture(image, i_coord);
}
