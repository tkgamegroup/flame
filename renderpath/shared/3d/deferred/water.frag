#include "../ubo_matrix.glsl"
#include "water.glsl"

layout (location = 0) in flat uint inWaterId;
layout (location = 1) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(ubo_matrix.view)));
	
	//vec3 normal = normalMatrix * normalize(vec3(L - R, 2.0 * eps, T - B));
	vec3 normal = normalMatrix * vec3(0, 1, 0);
	
	outAlbedoAlpha = vec4(1.0, 1.0, 1.0, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(1.0, 0.05, 0.0, 0.0);
}
