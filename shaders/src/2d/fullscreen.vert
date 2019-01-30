#if defined(VIEW_CAMERA)
#define UBO_MATRIX
#define UBO_MATRIX_SET 0
#define UBO_MATRIX_BINDING 0
#endif

#include "../resources.glsl"

#if defined(UV)
layout(location = 0) out vec2 o_uv;
#endif

#if defined(VIEW) || defined(VIEW_CAMERA)
layout(location = 1) out vec3 o_view;
layout(push_constant) uniform PushConstant
{
	float tan_hf_fovy;
	float aspect;
}p_camera;
#endif

void main()
{
	vec2 v = vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2);
#if defined(UV)
	o_uv = v;
#endif
	v = v * 2.0 - 1.0;
	gl_Position = vec4(v, 1, 1);
#if defined(VIEW)
	o_view = vec3(v * vec2(p_camera.tan_hf_fovy * p_camera.aspect, -p_camera.tan_hf_fovy), -1.0);
#elif defined(VIEW_CAMERA)
	o_view = transpose(mat3(u_matrix.view)) * vec3(v * vec2(p_camera.tan_hf_fovy * p_camera.aspect, -p_camera.tan_hf_fovy), -1.0);
#endif
}
