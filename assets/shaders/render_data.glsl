layout (set = 3, binding = 0) uniform RenderData
{
	float fovy;
	float aspect;
	float zNear;
	float zFar;
	vec3 camera_coord;
	float shadow_distance;
	uint csm_levels;
	float dumm1;
	vec2 dummy2;
	vec4 dummy3;
	mat4 view_inv;
	mat4 view;
	mat4 proj;
	mat4 proj_view;
}render_data;
