#include "..\math.glsl"

layout(push_constant) uniform PushConstant
{
	float spec;
}pc;

layout(binding = 0) uniform sampler2D img_source;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 n = inversePanorama(inTexcoord);
	
	vec4 result = vec4(0.0);
	
	for (float x = 0.0; x < PI * 2.0; x += 0.01)
		for (float y = -PI * 0.5; y < PI * 0.5; y += 0.01)
		{
			vec3 l = vec3(cos(x) * cos(y), sin(y), sin(x) * cos(y));
			float term = pow(max(0.0, dot(n, l)), pc.spec) * cos(y);
			result += vec4(texture(img_source, panorama(l)).rgb * term, term);
		}
		
    outColor = vec4(result.rgb / result.w, 1.0);
}
