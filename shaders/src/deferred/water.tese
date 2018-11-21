#include "../ubo_matrix.glsl"
#include "water.glsl"

layout(quads, equal_spacing, ccw) in;

layout (location = 0) in flat uint inWaterId[];
layout (location = 1) in vec2 inUV[];
 
layout (location = 0) out flat uint outWaterId;
layout (location = 1) out vec2 outUV;

void main()
{
	outWaterId = inWaterId[0];
	vec2 uv0 = mix(inUV[0], inUV[1], gl_TessCoord.x);
	vec2 uv1 = mix(inUV[3], inUV[2], gl_TessCoord.x);
	outUV = mix(uv0, uv1, gl_TessCoord.y);

	vec4 pos0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos1 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos0, pos1, gl_TessCoord.y);
	//pos.y += texture(heightMap, outUV).r * ubo_water.d[outWaterId].height;
	pos.xyz += ubo_water.d[outWaterId].coord;
	gl_Position = ubo_matrix.projView * pos;
}
