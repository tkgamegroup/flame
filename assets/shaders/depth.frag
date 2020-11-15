#include "depth.pll"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;

layout (location = 0) out float o_depth;

void main()
{
	MaterialInfo material = material_infos[i_mat_id];

	if (material.alpha_test > 0.0)
	{
		if (texture(maps[material.map_indices[0]], i_uv).a < material.alpha_test)
			discard;
	}

	if (pc.zNear == 0.0)
		o_depth = gl_FragCoord.z;
	else
		o_depth = pc.zNear / (pc.zFar + gl_FragCoord.z * (pc.zNear - pc.zFar));
}
