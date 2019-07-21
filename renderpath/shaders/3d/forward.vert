#define UBO_MATRIX
#define UBO_MATRIX_SET 0
#define UBO_MATRIX_BINDING 0

#include "resources.glsl"

#if defined(POS)
layout(location = POS_LOCATION) in vec3 a_pos;
#endif
#if defined(UV)
layout(location = UV_LOCATION) in vec2 a_uv;
layout(location = 0) out vec2 o_uv;
#endif
#if defined(NORMAL)
layout(location = NORMAL_LOCATION) in vec3 a_normal;
layout(location = 1) out vec3 o_normal;
#endif

void main()
{
#if defined(POS)
	gl_Position = u_matrix.proj * u_matrix.view * vec4(a_pos, 1.0);
#endif
#if defined(UV)
	o_uv = a_uv;
#endif
#if defined(NORMAL)
	o_normal = mat3(u_matrix.view) * a_normal;
#endif
}
