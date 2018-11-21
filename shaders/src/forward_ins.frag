layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inWorldCoord;

layout(location = 0) out vec4 outColor;

//layout(binding = 2) uniform sampler2D S_tex;

layout(set = 1, binding = 0) uniform U_matrix
{
	mat4 proj;
	mat4 view;
}u_matrix;

layout(binding = 0) uniform U_sky
{
	vec3 sun_dir;
	vec3 sun_color;
	float sun_size;
	vec3 sky_color;
	mat4 view_mat;
}u_sky;

struct PointLight
{
	vec3 pos;
	vec3 color;
};

layout(push_constant) uniform push_constant
{
	int point_light_count;
}pc;

layout(binding = 1) uniform U_light
{
	PointLight points[64];
}u_light;
		
void main()
{
	vec3 sun_dir_v = mat3(u_matrix.view) * u_sky.sun_dir;
	vec3 col = vec3(0.0);
	col += u_sky.sun_color * dot(inNormal, sun_dir_v);
	for (int i = 0; i < pc.point_light_count;i++)
	{
		vec3 dir = normalize(inWorldCoord - u_light.points[i].pos);
		col += u_light.points[i].color * dot(inNormal, dir);
	}
	outColor = vec4(col, 1.0);
}
