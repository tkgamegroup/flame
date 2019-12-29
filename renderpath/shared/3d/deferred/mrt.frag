#include "../material.glsl"

layout(location = 0) in flat uint inMaterialID;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;
		
void main()
{
	uint mapIndex;

	vec3 albedo;
	float alpha;
	mapIndex = ubo_material.material[inMaterialID].mapIndex & 0xff;
	if (mapIndex == 0)
	{
		uint v = ubo_material.material[inMaterialID].albedoAlphaCompress;
		albedo = vec3((v & 0xff) / 255.0, ((v >> 8) & 0xff) / 255.0, ((v >> 16) & 0xff) / 255.0);
		alpha = ((v >> 24) & 0xff) / 255.0;
	}
	else
	{
		vec4 v = texture(imgs_material[mapIndex - 1], inTexcoord);
		albedo = v.rgb;
		alpha = v.a;
	}
	
	vec3 normal = inNormal;
	mapIndex = (ubo_material.material[inMaterialID].mapIndex >> 8) & 0xff;
	if (mapIndex > 0)
	{
		vec4 v = texture(imgs_material[mapIndex - 1], inTexcoord);
		vec3 tn = normalize(v.xyz * 2.0 - 1.0);
		normal = normalize(mat3(-inTangent, cross(normal, -inTangent), normal) * tn);
	}

	float spec, roughness;
	mapIndex = (ubo_material.material[inMaterialID].mapIndex >> 16) & 0xff;
	if (mapIndex == 0)
	{
		uint v = ubo_material.material[inMaterialID].specRoughnessCompress;
		spec = (v & 0xff) / 255.0;
		roughness = ((v >> 8) & 0xff) / 255.0;
	}
	else
	{
		vec4 v = texture(imgs_material[mapIndex - 1], inTexcoord);
		spec = v.r;
		roughness = v.g;
	}
	
	outAlbedoAlpha = vec4(albedo, alpha);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(spec, roughness, 0.0, 0.0);
}