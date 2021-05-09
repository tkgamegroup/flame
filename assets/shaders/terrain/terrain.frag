#ifndef DEFERRED
#include "forward.pll"
#include "../shading.glsl"
#else
#include "../math.glsl"
#include "gbuffer.pll"
#endif

layout(location = 0) in flat uint i_id;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
#ifndef DEFERRED
layout(location = 3) in vec3 i_coordw;
layout(location = 4) in vec3 i_coordv;
#endif

#ifndef DEFERRED
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;
#endif

void main()
{
#ifdef MAT
	TerrainInfo terrain = terrain_infos[i_id];
	MaterialInfo material = material_infos[terrain.material_id];

	vec3 N = i_normal;
#ifndef DEFERRED
	vec3 V = i_coordv;
#endif

	MAT_FILE

#ifdef DEFERRED
	o_res_col_met = vec4(color.rgb, metallic);
	o_res_nor_rou = vec4(N * 0.5 + vec3(0.5), roughness);
#endif

#else

#ifdef PICKUP
	o_color = pack_uint_to_v4(pc.i[0]);
#else
	o_color = pc.f;
#endif

#endif
}
