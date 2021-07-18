#ifdef MAKE_DSL
#define RENDER_DATA_SET 0
#endif

layout (set = RENDER_DATA_SET, binding = 0) uniform RenderData
{
	vec3 fog_color;
	float sky_intensity;
	float sky_rad_levels;

	float zNear;
	float zFar;
	
	vec3 camera_coord;
	vec3 camera_dir;

	mat4 view;
	mat4 view_inv;
	mat4 proj;
	mat4 proj_inv;
	mat4 proj_view;
	
	vec4 frustum_planes[6];
}render_data;
