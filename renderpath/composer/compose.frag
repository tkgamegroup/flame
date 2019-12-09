in vec4 i_color;
in vec2 i_uv;
in uint i_id;

out vec4 o_color{sc:sa dc:1msa da:1};

sampler2D images[64];

void main()
{
	o_color = i_color * texture(images[i_id], i_uv);
}
