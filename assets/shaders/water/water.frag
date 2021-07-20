#include "water.pll"
#include "../shading.glsl"

layout(location = 0) in flat uint i_idx;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_coordw;

layout(location = 0) out vec4 o_color;

void main()
{
#ifdef MAT
	MaterialInfo material = material_infos[water_infos[i_idx].material_id];
	
	MAT_FILE
#else

#endif
}
