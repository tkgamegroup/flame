vec3 get_ssr(vec3 N, vec3 V, vec3 view_pos)
{
	vec3 R = reflect(-V, N);
	vec3 hit_pos = view_pos.xyz;

	return V;
}