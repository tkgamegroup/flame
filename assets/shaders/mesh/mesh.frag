#include "../math.glsl"

#ifndef DEFERRED
#include "../forward.pll"
#ifdef MAT_FILE
#include "../shading.glsl"
#endif
#else
#include "../gbuffer.pll"
#endif

layout(location = 0) in flat uint i_matid;
layout(location = 1) in	     vec2 i_uv;
#ifndef DEPTH_PASS
layout(location = 2) in      vec3 i_normal;
layout(location = 3) in      vec3 i_coordw;
#endif

#ifndef DEPTH_PASS
#ifndef DEFERRED
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;
#endif
#endif

void main()
{
#include "../editor_mat.glsl"
}
