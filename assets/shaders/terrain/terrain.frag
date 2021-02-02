#include "terrain.pll"
#include "../shading.glsl"

layout (location = 0) in flat uint i_tid;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coordw;
layout (location = 3) in vec3 i_coordv;
layout (location = 4) in vec3 i_normal;

layout(location = 0) out vec4 o_color;

void main()
{
#ifdef MAT
	TerrainInfo terrain = terrain_infos[i_tid];
	MaterialInfo material = material_infos[terrain.material_id];
	
	vec3 N = normalize(i_normal);
	vec3 V = normalize(i_coordv);

	MAT_FILE
#else

#ifdef PICKUP
	uint id = pc.id;
	o_color[0] = id & 0xff;
	id >>= 8;
	o_color[1] = id & 0xff;
	id >>= 8;
	o_color[2] = id & 0xff;
	id >>= 8;
	o_color[3] = id & 0xff;
#else
	o_color = vec4(0.0, 1.0, 0.0, 1.0);
#endif

#endif
}
