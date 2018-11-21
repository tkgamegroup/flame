#include "terrain.glsl"

layout (location = 0) out vec2 outUV;

void main(void)
{
	vec2 v[4] = {
		vec2(0.0, 1.0),
		vec2(0.0, 0.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0)
	};
	outUV = vec2((gl_InstanceIndex % ubo_terrain.count.x) + v[gl_VertexIndex].x, 
		(gl_InstanceIndex / ubo_terrain.count.x) + v[gl_VertexIndex].y);
	gl_Position = vec4(vec3((outUV.x - 0.5) * ubo_terrain.size, 0.0, 
		(outUV.y - 0.5) * ubo_terrain.size) + ubo_terrain.coord, 1.0);
	outUV /= ubo_terrain.count;
}
