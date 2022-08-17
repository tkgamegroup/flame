layout (set = SET, binding = 0) uniform Camera
{
	float zNear;
	float zFar;
	float fovy;
	float tan_hf_fovy;

	vec2 viewport;
	
	vec3 coord;
	vec3 front;
	vec3 right;
	vec3 up;

	mat4 view;
	mat4 view_inv;
	mat4 proj;
	mat4 proj_inv;
	mat4 proj_view;
	mat4 proj_view_inv;
	
	vec4 frustum_planes[6];

	uint time;
}camera;
