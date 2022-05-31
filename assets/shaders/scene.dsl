layout (set = SET, binding = 0) uniform Scene
{
	float sky_intensity;
	float sky_rad_levels;
	vec3 fog_color;

	float zNear;
	float zFar;

	vec2 viewport;
	
	vec3 camera_coord;
	vec3 camera_dir;

	mat4 view;
	mat4 view_inv;
	mat4 proj;
	mat4 proj_inv;
	mat4 proj_view;
	mat4 proj_view_inv;
	
	vec4 frustum_planes[6];

	uint time;
}scene;
