#include "depth.pll"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;

layout (location = 0) out float o_depth;

void main()
{
#ifdef MAT
	MaterialInfo material = material_infos[i_mat_id];
	MAT_FILE
#endif

	if (pc.zNear == 0.0)
		o_depth = gl_FragCoord.z;
	else
		o_depth = pc.zNear / (pc.zFar + gl_FragCoord.z * (pc.zNear - pc.zFar));
}
