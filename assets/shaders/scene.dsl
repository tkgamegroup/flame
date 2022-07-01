layout (set = SET, binding = 0) uniform Scene
{
	float zNear;
	float zFar;
	float fovy;
	float tan_hf_fovy;

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
