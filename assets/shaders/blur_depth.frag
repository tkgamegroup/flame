layout (location = 0) in vec2 i_uv;

layout (location = 0) out float o_depth;

layout (set = 0, binding = 0) uniform sampler2D image;

layout (push_constant) uniform PushConstantT
{
	float pxsz;
}pc;

void main()
{
	ivec2 c = ivec2(gl_FragCoord.xy);
	float res = texelFetch(image, c, 0).r;
	res *= 0.218817;
#ifdef H
	res += texture(image, i_uv + vec2(-5 * pc.pxsz, 0)).r * 0.005086;
	res += texture(image, i_uv + vec2(-4 * pc.pxsz, 0)).r * 0.019711;
	res += texture(image, i_uv + vec2(-3 * pc.pxsz, 0)).r * 0.056512;
	res += texture(image, i_uv + vec2(-2 * pc.pxsz, 0)).r * 0.119895;
	res += texture(image, i_uv + vec2(-1 * pc.pxsz, 0)).r * 0.188263;
	res += texture(image, i_uv + vec2(+1 * pc.pxsz, 0)).r * 0.188263;
	res += texture(image, i_uv + vec2(+2 * pc.pxsz, 0)).r * 0.119895;
	res += texture(image, i_uv + vec2(+3 * pc.pxsz, 0)).r * 0.056512;
	res += texture(image, i_uv + vec2(+4 * pc.pxsz, 0)).r * 0.019711;
	res += texture(image, i_uv + vec2(+5 * pc.pxsz, 0)).r * 0.005086;
#endif
#ifdef V
	res += texture(image, i_uv + vec2(0, -5 * pc.pxsz)).r * 0.005086;
	res += texture(image, i_uv + vec2(0, -4 * pc.pxsz)).r * 0.019711;
	res += texture(image, i_uv + vec2(0, -3 * pc.pxsz)).r * 0.056512;
	res += texture(image, i_uv + vec2(0, -2 * pc.pxsz)).r * 0.119895;
	res += texture(image, i_uv + vec2(0, -1 * pc.pxsz)).r * 0.188263;
	res += texture(image, i_uv + vec2(0, +1 * pc.pxsz)).r * 0.188263;
	res += texture(image, i_uv + vec2(0, +2 * pc.pxsz)).r * 0.119895;
	res += texture(image, i_uv + vec2(0, +3 * pc.pxsz)).r * 0.056512;
	res += texture(image, i_uv + vec2(0, +4 * pc.pxsz)).r * 0.019711;
	res += texture(image, i_uv + vec2(0, +5 * pc.pxsz)).r * 0.005086;
#endif
	o_depth = res;
}
