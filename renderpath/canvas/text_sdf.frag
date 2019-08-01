float median(vec3 v) 
{
    return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
}
 
void main()
{
	float sig_dist = median(texture(images[in_id], in_uv).rgb) - 0.5;
	sig_dist *= dot(pc.sdf_range, 0.5 / fwidth(in_uv));
	out_color = vec4(in_color.rgb, in_color.a * clamp(sig_dist + 0.5, 0.0, 1.0));
}
