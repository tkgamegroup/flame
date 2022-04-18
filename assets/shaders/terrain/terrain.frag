#include "../math.glsl"

#ifndef DEFERRED
#include "../forward.pll"
#ifdef MAT_FILE
#include "../shading.glsl"
#endif
#else
#include "../gbuffer.pll"
#endif

layout(location = 0) in flat uint i_id;
layout(location = 1) in flat uint i_matid;
layout(location = 2) in		 vec2 i_uv;
#ifndef DEPTH_PASS
layout(location = 3) in		 vec3 i_normal;
layout(location = 4) in		 vec3 i_tangent;
#ifndef DEFERRED
layout(location = 5) in		 vec3 i_coordw;
#endif
#endif

#ifndef DEPTH_PASS
#ifndef DEFERRED
layout(location = 0) out vec4 o_color;
#else
layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;
#endif
vec3 textureVariationTiling(int map_id, vec2 uv)
{
	float k = 0;
	if (material_misc.random_map_id != -1)
		k = texture(material_maps[material_misc.random_map_id], 0.005 * uv).r;
    
    vec2 duvdx = dFdx(uv);
    vec2 duvdy = dFdy(uv);
    
    float l = k * 8.0;
    float f = fract(l);

    float ia = floor(l);
    float ib = ia + 1.0;
    
    vec2 offa = sin(vec2(3.0, 7.0) * ia);
    vec2 offb = sin(vec2(3.0, 7.0) * ib);

    vec3 cola = textureGrad(material_maps[map_id], uv + offa, duvdx, duvdy).rgb;
    vec3 colb = textureGrad(material_maps[map_id], uv + offb, duvdx, duvdy).rgb;
    
    return mix(cola, colb, smoothstep(0.2, 0.8, f - 0.1 * sum(cola - colb)));
}
#endif

void main()
{
#include "../editor_mat.glsl"
}
