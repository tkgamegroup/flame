#include "terrain.glsl"

layout(quads, equal_spacing, cw) in;

layout (binding = 1) uniform sampler2D displacement_map; 

layout (location = 0) in vec2 inUV[];
 
layout (location = 0) out vec2 outUV;

void main()
{
	outUV = 
		mix(
			mix(inUV[0], inUV[1], gl_TessCoord.x), 
			mix(inUV[3], inUV[2], gl_TessCoord.x), 
			gl_TessCoord.y
		);

	vec4 pos = 
		mix(
			mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x), 
			mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x), 
			gl_TessCoord.y
		);

	pos.y += texture(displacement_map, outUV).r * ubo_terrain.height;

	gl_Position = ubo_terrain.proj_matrix * ubo_terrain.view_matrix * pos;
}
