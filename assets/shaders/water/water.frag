#include "../shading.glsl"

layout(location = 0) in flat uint i_idx;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_coordw;

layout(location = 0) out vec4 o_color;

void main()
{
#ifdef MAT_FILE
	MaterialInfo material = material.infos[water_infos[i_idx].material_id];
	
	#inlcude MAT_FILE

	float d1 = texture(img_depth, gl_FragCoord.xy / scene.viewport).r;
	d1 = d1 * 0.5 + 0.5;
	float d2 = gl_FragCoord.z;
	d2 = d2 * 0.5 + 0.5;
	d1 = linear_depth(scene.zNear, scene.zFar, d1);
	d2 = linear_depth(scene.zNear, scene.zFar, d2);
	float reduction = 1.0 - (d1 - d2) * 20.0 / scene.zFar;
	reduction = clamp(reduction * reduction, 0.0, 0.99);

	float foam = d2 + 0.2 > d1 ? 1.0 - (d1 - d2) / 0.2 : 0.0;
	o_color = vec4(shading(i_coordw, N, 0.0, mix(vec3(0.01, 0.08, 0.11), vec3(1.0), foam), vec3(0.04), 0.05, 1.0), reduction);
#else

#endif
}
