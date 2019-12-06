in vec4 i_color;
in vec2 i_uv;
in uint i_id;

out vec4 o_color{sc:sa dc:1msa da:1};

pushconstant
{
	vec2 scale;
	vec2 sdf_range;
}pc;

sampler2D images[64];

float median(vec3 v) 
{
    return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
}
 
void main()
{
	float sig_dist = median(texture(images[i_id], i_uv).rgb) - 0.5;
	sig_dist *= dot(pc.sdf_range, 0.5 / fwidth(i_uv));
	o_color = vec4(i_color.rgb, i_color.a * clamp(sig_dist + 0.5, 0.0, 1.0));
}
