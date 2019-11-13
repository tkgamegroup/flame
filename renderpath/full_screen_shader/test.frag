in vec2 i_coord;

out vec4 o_color;

void main()
{
	o_color = vec4(0.8 * i_coord.x, 0.9 * i_coord.y, 0.2, 1.0);
}
