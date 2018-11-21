layout(binding = 0) uniform sampler2D img_source;

layout(location = 0) out vec4 outColor;

#if defined(DEBUG)
const float gamma = 1.0;
#else
const float gamma = 0.45455;
#endif

void main()
{
	outColor = vec4(pow(texture(img_source, gl_FragCoord.xy).rgb, vec3(gamma)), 1.0);
}
