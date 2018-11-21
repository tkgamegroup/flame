#include "../ubo_matrix.glsl"
#include "water.glsl"

layout (vertices = 4) out;
 
layout (location = 0) in flat uint inWaterId[];
layout (location = 1) in vec2 inUV[];
 
layout (location = 0) out flat uint outWaterId[4];
layout (location = 1) out vec2 outUV[4];
 
float screenSpaceTessFactor(vec4 p0, vec4 p1)
{
	vec4 midPoint = 0.5 * (p0 + p1);
	float radius = distance(p0, p1) / 2.0;

	vec4 v0 = ubo_matrix.view * midPoint;

	vec4 clip0 = (ubo_matrix.proj * (v0 - vec4(radius, vec3(0.0))));
	vec4 clip1 = (ubo_matrix.proj * (v0 + vec4(radius, vec3(0.0))));

	clip0 /= clip0.w;
	clip1 /= clip1.w;

	clip0.xy *= ubo_matrix.viewportDim;
	clip1.xy *= ubo_matrix.viewportDim;
	
	return clamp(distance(clip0, clip1) / ubo_water.d[inWaterId[0]].block_size * ubo_water.d[inWaterId[0]].tessellation_factor, 1.0, 64.0);
}

bool frustumCheck()
{
	vec2 uv = (inUV[0] + inUV[1] + inUV[2] + inUV[3]) * 0.25;
	
	const float radius = max(ubo_water.d[inWaterId[0]].block_size, ubo_water.d[inWaterId[0]].height);
	vec4 pos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position + gl_in[3].gl_Position) * 0.25;
	//pos.y += texture(heightMap, uv).r * ubo_water.d[inWaterId[0]].height;
	pos.xyz += ubo_water.d[inWaterId[0]].coord;
	pos = ubo_matrix.projView * pos;
	pos = pos / pos.w;

	for (int i = 0; i < 6; i++) 
	{
		if (dot(pos, ubo_matrix.frustumPlanes[i]) + radius < 0.0)
			return false;
	}
	return true;
}

void main()
{
	outWaterId[gl_InvocationID] = inWaterId[0];
	outUV[gl_InvocationID] = inUV[gl_InvocationID];

	if (gl_InvocationID == 0)
	{
		if (!frustumCheck())
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
		}
		else
		{
			
			if (ubo_water.d[inWaterId[0]].tessellation_factor > 0.0)
			{
				gl_TessLevelOuter[0] = screenSpaceTessFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
				gl_TessLevelOuter[1] = screenSpaceTessFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
				gl_TessLevelOuter[2] = screenSpaceTessFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
				gl_TessLevelOuter[3] = screenSpaceTessFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);
				gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
				gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
			}
			else
			{
				gl_TessLevelInner[0] = 1.0;
				gl_TessLevelInner[1] = 1.0;
				gl_TessLevelOuter[0] = 1.0;
				gl_TessLevelOuter[1] = 1.0;
				gl_TessLevelOuter[2] = 1.0;
				gl_TessLevelOuter[3] = 1.0;
			}
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
} 
