#include "..\math.h"

struct Material
{
	uint albedoAlphaCompress;
	uint specRoughnessCompress;

	uint mapIndex;
	
	uint dummy;
};

layout(set = 1, binding = 0) uniform ubo_material_
{
	Material material[256];
}ubo_material;

layout(set = 1, binding = 1) uniform sampler2D imgs_material[256];

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in flat uint inMaterialID;

layout(location = 0) out float outExp;

highp float map_01(float x, float v0, float v1)
{
	return (x - v0) / (v1 - v0);
}
		
void main()
{
	/*
	uint mapIndex = ubo_material.material[inMaterialID].mapIndex & 0xff;
	if (mapIndex != 0)
	{
		vec4 v = texture(imgs_material[mapIndex - 1], inTexcoord);
		if (v.a < 0.5)
			discard;
	}
	*/

	// Exponential is a configurable constant for approximation.
	// Generally a higher Exponential means greater difference in depths.
	// Because of this there will be less error, but we may run out of precision.
	outExp = exp(/*Light.Exponential*/esm_factor * (gl_FragCoord.z / gl_FragCoord.w));
	//outExp = gl_FragCoord.z / gl_FragCoord.w;
	//outExp = gl_FragCoord.z;
}