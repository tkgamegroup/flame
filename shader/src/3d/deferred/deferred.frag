#include "..\math.glsl"

#if !defined(USE_PHGON) && !defined(USE_PBR)
#define USE_PHONG
#endif

//#define DEBUG_NORMAL
//#define DEBUG_WORLD_COORD

#include "../ubo_constant.glsl"
#include "../ubo_matrix.glsl"

layout(binding = 6) uniform sampler2D img_depth;
layout(binding = 7) uniform sampler2D img_albedo_alpha;
layout(binding = 8) uniform sampler2D img_normal_height;
layout(binding = 9) uniform sampler2D img_spec_roughness;

#include "../ubo_light.glsl"

layout(binding = 11) uniform sampler2D img_envr;

#include "../ubo_ambient.glsl"

layout(binding = 13) uniform sampler2D img_ao;

#include "../shadow.glsl"

layout(location = 0) in vec3 inViewDir;

layout(location = 0) out vec4 outColor;

float F_schlick(float f0, float cita)
{
	return f0 + (1.0 - f0) * pow(1.0 - cita, 5.0);
}

float G1_schlick(float cita, float k)
{
	return cita / (cita * (1.0 - k) + k);
}

float G_schlick(float alpha, float nl, float nv)
{
	float k = 0.8 + 0.5 * alpha;
	k *= k * 0.5;
	return G1_schlick(nl, k) * G1_schlick(nv, k);
}

float D_GGX(float alpha, float nh)
{
	float r = alpha / (nh * (alpha * alpha - 1.0) + 1.0);
	return r * r * PI_INV;
}

vec3 brdf(vec3 V, vec3 L, vec3 N, float roughness, float spec, vec3 albedo, vec3 lightColor)
{
	vec3 H = normalize(L + V);
	float nl = dot(N, L);
	float nv = dot(N, V);
	float alpha = pow(1.0 - (1.0 - roughness) * 0.7, 6.0);
	return (albedo + D_GGX(alpha, dot(N, H)) * F_schlick(spec, dot(L, H)) * G_schlick(alpha, nl, nv) / (4.0 * nl * nv)) * lightColor;
}

float specularOcclusion(float dotNV, float ao, float smothness)
{
    return clamp(pow(dotNV + ao, smothness) - 1.0 + ao, 0.0, 1.0);
}

highp float map_01(float x, float v0, float v1)
{
	return (x - v0) / (v1 - v0);
}

void main()
{
	vec3 viewDir = normalize(inViewDir);

	float inDepth = texture(img_depth, gl_FragCoord.xy).r;
	if (inDepth == 1.0)
	{
		outColor = vec4(textureLod(img_envr, panorama(mat3(ubo_matrix.viewInv) * viewDir), 0).rgb, 1.0);
		return;
	}
		
	float linerDepth = LinearDepthPerspective(inDepth, ubo_constant.near, ubo_constant.far);
		
	vec3 coordView = inViewDir * (-linerDepth / inViewDir.z);
	vec4 coordWorld = ubo_matrix.viewInv * vec4(coordView, 1.0);
#if defined(DEBUG_WORLD_COORD)
	outColor = vec4(coordWorld.xyz, 1.0);
	return;
#endif
	
	vec4 inAlbedoAlpha = texture(img_albedo_alpha, gl_FragCoord.xy);
	vec4 inNormalHeight = texture(img_normal_height, gl_FragCoord.xy);
	vec4 inSpecRoughness = texture(img_spec_roughness, gl_FragCoord.xy);
	
	vec3 albedo = inAlbedoAlpha.rgb;
	float spec = inSpecRoughness.r;
	albedo *= 1.0 - spec;
	float roughness = inSpecRoughness.g;
	float smothness = 1.0 - roughness;
	
	vec3 normal = normalize(inNormalHeight.xyz * 2.0 - 1.0);
#if defined(DEBUG_NORMAL)
	outColor = vec4(normal, 1.0);
	return;
#endif
	
	vec3 lightSumColor = vec3(0.0);
	for (int i = 0; i < ubo_light.count; i++)
	{
		Light light = ubo_light.lights[i];

		float visibility = 1.0;
		/*
		{
			int shadowId = int(light.color.a);
			if (shadowId != -1)
			{
				vec4 shadowCoord = ubo_shadow.matrix[shadowId] * coordWorld; 
				shadowCoord /= shadowCoord.w;
				if (shadowCoord.z >= 0.0 && shadowCoord.z <= 1.0)
				{
					float occluder = texture(img_shadow, vec3(shadowCoord.xy * 0.5 + 0.5, shadowId * 6)).r;
					float reciever = shadowCoord.z;
					//visibility = clamp(occluder * exp(-esm_factor * reciever), 0.0, 1.0);
					visibility = occluder < exp(esm_factor * reciever) ? 0.0 : 1.0;
					//visibility = (occluder / exp(esm_factor * reciever)) / 8.0;
					//visibility = (exp(esm_factor * reciever) - occluder) / 8.0;
					//visibility = occluder < reciever ? 0.0 : 1.0;
				}

				//lightSumColor = vec3(visibility); break;
			}
		}
		*/
		if (visibility == 0.0) continue;
		
		vec3 lightColor = light.color.xyz * visibility;
		vec3 lightDir = light.coord.xyz;
		if (light.coord.w == 0.0)
		{
			lightDir = (ubo_matrix.view * vec4(lightDir, 0.0)).xyz;
		}
		else
		{
			lightDir = (ubo_matrix.view * vec4(lightDir, 1.0)).xyz - coordView;
			lightColor /= lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z;
		}
		lightDir = normalize(lightDir);
		float nl = dot(normal, lightDir);
		if (nl < 0.0) continue;

		if (light.coord.w == 2.0)
		{
			float spotVal = dot(-lightDir, vec3(ubo_matrix.view * vec4(light.spotData.xyz, 0.0)));
			spotVal -= light.spotData.w;
			if (spotVal < 0.0) continue;
			float k = spotVal / (1.0 - light.spotData.w);
			lightColor *= k * k;
		}
#if defined(USE_PHONG)
		vec3 r = reflect(-lightDir, normal);
		float vr = max(dot(r, -viewDir), 0.0);
		lightSumColor += albedo * lightColor * nl + spec * pow(vr, (1.0 - roughness) * 128.0) * lightColor;
#elif defined(USE_PBR)
		lightSumColor += brdf(-viewDir, lightDir, normal, roughness, spec, albedo, lightColor) * nl;
#endif
	}
	
	vec3 color = lightSumColor;
#if defined(USE_IBL)
	mat3 matrixViewInv3 = mat3(ubo_matrix.viewInv);
	vec3 irradiance = albedo * textureLod(img_envr, panorama(matrixViewInv3 * normal), ubo_ambient.envr_max_mipmap).rgb;
	vec3 radiance = smothness * F_schlick(spec, dot(-viewDir, normal)) * 
	textureLod(img_envr, panorama(matrixViewInv3 * reflect(viewDir, normal)), roughness * ubo_ambient.envr_max_mipmap).rgb;
	color += ubo_ambient.color * (irradiance + radiance);
#else
	color += ubo_ambient.color * albedo;
#endif
	
#if defined(USE_FOG)
	float fog = clamp(exp2(-0.01 * 0.01 * linerDepth * linerDepth * 1.442695), 0.0, 1.0);
	color = mix(ubo_ambient.fogColor.rgb, color, fog);
#endif
	outColor = vec4(color, 1.0);
}
