layout(location = 0) in vec3 inViewDir;

layout(location = 0) out vec4 outColor0;
layout(location = 1) out vec4 outColor1;

layout(binding = 1) uniform ubo_sky_
{
	vec4 sun_dir;
	vec4 sun_color;
	vec4 sun_size;
	vec4 sky_color;
	mat4 view_mat;
}ubo_sky;
		
void main()
{
	vec4 col = vec4(vec3(ubo_sky.sky_color) +
		vec3(ubo_sky.sun_color) * 
		pow(
		max(0.0, dot(vec3(ubo_sky.sun_dir), transpose(mat3(ubo_sky.view_mat)) * normalize(inViewDir)))
		, ubo_sky.sun_size.x)
		, 1.0);
	outColor0 = col;
	outColor1 = col;
}
