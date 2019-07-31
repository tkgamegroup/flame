layout(binding = 0) uniform sampler2D images[64];

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in flat uint inID;

layout(location = 0) out vec4 fColor;

float median(vec3 v) 
{
    return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
}
 
void main()
{
	float sig_dist = median(texture(images[inID], inUV).rgb) - 0.5;
	sig_dist *= dot(pc.sdf_range, 0.5 / fwidth(inUV));
	fColor = vec4(inColor.rgb, inColor.a * clamp(sig_dist + 0.5, 0.0, 1.0));
}
