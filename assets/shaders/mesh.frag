#include "mesh.pll"

#include "shading.glsl"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coordw;
layout (location = 3) in vec3 i_coordv;
layout (location = 4) in vec3 i_normal;
layout (location = 5) in vec4 i_debug;

layout (location = 0) out vec4 o_color;

void main()
{
	vec3 res = vec3(0.0);

	MaterialInfo material = material_infos[i_mat_id];

	vec3 N = normalize(i_normal);
	vec3 V = normalize(i_coordv);
	
#ifdef MAT
	#include MAT_FILE
#else
	o_color = vec4(0.0, 1.0, 0.0, 1.0);
#endif
}
