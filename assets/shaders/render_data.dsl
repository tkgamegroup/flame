#ifdef MAKE_DSL
#define RENDER_DATA_SET 0
#endif

layout (set = RENDER_DATA_SET, binding = 0) uniform RenderData
{
	float shadow_distance;
	uint csm_levels;

	vec2 fb_size;
	
	float fovy;
	float aspect;
	float zNear;
	float zFar;
	
	vec3 camera_coord;
	mat3 camera_dirs;

	mat4 view_inv;
	mat4 view;
	mat4 proj;
	mat4 proj_view;
	
	vec4 frustum_planes[6];
	
	int sky_tex_id;
}render_data;
