layout (set = RENDER_DATA_SET, binding = 0) uniform RenderData
{
	float fovy;
	float aspect;
	float zNear;
	float zFar;
	vec3 camera_coord;
	float dummy1;
	vec4 frustum_planes[6];

	vec2 fb_size;
	float shadow_distance;
	uint csm_levels;
	vec4 dummy3[3];

	mat4 view_inv;
	mat4 view;
	mat4 proj;
	mat4 proj_view;
}render_data;
