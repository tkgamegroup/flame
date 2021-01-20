#include "terrain.pll"
#include "../shading.glsl"

layout (location = 0) in flat uint i_idx;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coordw;
layout (location = 3) in vec3 i_coordv;
layout (location = 4) in vec3 i_normal;

layout(location = 0) out vec4 o_color;

void main()
{
#ifdef MAT
	TerrainInfo terrain = terrain_infos[i_idx];
	MaterialInfo material = material_infos[terrain.material_id];
	
	vec3 N = normalize(i_normal);
	vec3 V = normalize(i_coordv);

	MAT_FILE
#else
	o_color = vec4(0.0, 1.0, 0.0, 1.0);
#endif
}
