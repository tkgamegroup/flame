layout (set = 3, binding = 0) uniform CameraData
{
	float fovy;
	float aspect;
	float zNear;
	float zFar;
	vec3 coord;
	int dummy1;
	ivec4 dummy2;
	ivec4 dummy3;
	mat4 view_inv;
	mat4 view;
	mat4 proj;
	mat4 proj_view;
}camera_data;
