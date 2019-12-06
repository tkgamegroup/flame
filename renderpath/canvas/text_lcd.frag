in vec4 i_color;
in vec2 i_uv;
in uint i_id;

out vec4 o_color{sc:s1c dc:1ms1c da:1};

sampler2D images[64];

void main()
{
	o_color0 = i_color;
	o_color1 = texture(images[i_id], i_uv) * i_color.a;
}
