#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 o_color;

layout (set = 0, binding = 0) uniform sampler2D image;

void main()
{
	ivec2 c = ivec2(gl_FragCoord.xy);

#ifdef R1
	vec3 res = texelFetch(image, c, 0).rgb * 0.904419;
#ifdef H
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.04779;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.04779;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.04779;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.04779;
#endif
#endif

#ifdef R2
	vec3 res = texelFetch(image, c, 0).rgb * 0.52495;
#ifdef H
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.015885;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.015885;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.015885;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.221463;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.015885;
#endif
#endif

#ifdef R3
	vec3 res = texelFetch(image, c, 0).rgb * 0.382925;
#ifdef H
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.005977;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.005977;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.005977;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.24173;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.060598;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.005977;
#endif
#endif

#ifdef R4
	vec3 res = texelFetch(image, c, 0).rgb * 0.279015;
#ifdef H
	res += texelFetch(image, c + ivec2(-4, 0), 0).rgb * 0.005556;
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.030863;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.104916;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.218504;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.218504;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.104916;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.030863;
	res += texelFetch(image, c + ivec2(+4, 0), 0).rgb * 0.005556;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -4), 0).rgb * 0.005556;
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.030863;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.104916;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.218504;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.218504;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.104916;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.030863;
	res += texelFetch(image, c + ivec2(0, +4), 0).rgb * 0.005556;
#endif
#endif

#ifdef R5
	vec3 res = texelFetch(image, c, 0).rgb * 0.218817;
#ifdef H
	res += texelFetch(image, c + ivec2(-5, 0), 0).rgb * 0.005086;
	res += texelFetch(image, c + ivec2(-4, 0), 0).rgb * 0.019711;
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.056512;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.119895;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.188263;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.188263;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.119895;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.056512;
	res += texelFetch(image, c + ivec2(+4, 0), 0).rgb * 0.019711;
	res += texelFetch(image, c + ivec2(+5, 0), 0).rgb * 0.005086;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -5), 0).rgb * 0.005086;
	res += texelFetch(image, c + ivec2(0, -4), 0).rgb * 0.019711;
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.056512;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.119895;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.188263;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.188263;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.119895;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.056512;
	res += texelFetch(image, c + ivec2(0, +4), 0).rgb * 0.019711;
	res += texelFetch(image, c + ivec2(0, +5), 0).rgb * 0.005086;
#endif
#endif

#ifdef R6
	vec3 res = texelFetch(image, c, 0).rgb * 0.179788;
#ifdef H
	res += texelFetch(image, c + ivec2(-6, 0), 0).rgb * 0.004644;
	res += texelFetch(image, c + ivec2(-5, 0), 0).rgb * 0.014195;
	res += texelFetch(image, c + ivec2(-4, 0), 0).rgb * 0.03541;
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.072087;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.119775;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.162429;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.162429;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.119775;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.072087;
	res += texelFetch(image, c + ivec2(+4, 0), 0).rgb * 0.03541;
	res += texelFetch(image, c + ivec2(+5, 0), 0).rgb * 0.014195;
	res += texelFetch(image, c + ivec2(+6, 0), 0).rgb * 0.004644;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -6), 0).rgb * 0.004644;
	res += texelFetch(image, c + ivec2(0, -5), 0).rgb * 0.014195;
	res += texelFetch(image, c + ivec2(0, -4), 0).rgb * 0.03541;
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.072087;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.119775;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.162429;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.162429;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.119775;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.072087;
	res += texelFetch(image, c + ivec2(0, +4), 0).rgb * 0.03541;
	res += texelFetch(image, c + ivec2(0, +5), 0).rgb * 0.014195;
	res += texelFetch(image, c + ivec2(0, +6), 0).rgb * 0.004644;
#endif
#endif

#ifdef R7
	vec3 res = texelFetch(image, c, 0).rgb * 0.152499;
#ifdef H
	res += texelFetch(image, c + ivec2(-7, 0), 0).rgb * 0.00425;
	res += texelFetch(image, c + ivec2(-6, 0), 0).rgb * 0.010989;
	res += texelFetch(image, c + ivec2(-5, 0), 0).rgb * 0.024548;
	res += texelFetch(image, c + ivec2(-4, 0), 0).rgb * 0.04738;
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.079014;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.113855;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.141755;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.141755;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.113855;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.079014;
	res += texelFetch(image, c + ivec2(+4, 0), 0).rgb * 0.04738;
	res += texelFetch(image, c + ivec2(+5, 0), 0).rgb * 0.024548;
	res += texelFetch(image, c + ivec2(+6, 0), 0).rgb * 0.010989;
	res += texelFetch(image, c + ivec2(+7, 0), 0).rgb * 0.00425;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -7), 0).rgb * 0.00425;
	res += texelFetch(image, c + ivec2(0, -6), 0).rgb * 0.010989;
	res += texelFetch(image, c + ivec2(0, -5), 0).rgb * 0.024548;
	res += texelFetch(image, c + ivec2(0, -4), 0).rgb * 0.04738;
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.079014;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.113855;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.141755;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.141755;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.113855;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.079014;
	res += texelFetch(image, c + ivec2(0, +4), 0).rgb * 0.04738;
	res += texelFetch(image, c + ivec2(0, +5), 0).rgb * 0.024548;
	res += texelFetch(image, c + ivec2(0, +6), 0).rgb * 0.010989;
	res += texelFetch(image, c + ivec2(0, +7), 0).rgb * 0.00425;
#endif
#endif

#ifdef R8
	vec3 res = texelFetch(image, c, 0).rgb * 0.128135;
#ifdef H
	res += texelFetch(image, c + ivec2(-8, 0), 0).rgb * 0.00472;
	res += texelFetch(image, c + ivec2(-7, 0), 0).rgb * 0.010233;
	res += texelFetch(image, c + ivec2(-6, 0), 0).rgb * 0.020009;
	res += texelFetch(image, c + ivec2(-5, 0), 0).rgb * 0.035289;
	res += texelFetch(image, c + ivec2(-4, 0), 0).rgb * 0.056137;
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.080549;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.104247;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.121694;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.121694;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.104247;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.080549;
	res += texelFetch(image, c + ivec2(+4, 0), 0).rgb * 0.056137;
	res += texelFetch(image, c + ivec2(+5, 0), 0).rgb * 0.035289;
	res += texelFetch(image, c + ivec2(+6, 0), 0).rgb * 0.020009;
	res += texelFetch(image, c + ivec2(+7, 0), 0).rgb * 0.010233;
	res += texelFetch(image, c + ivec2(+8, 0), 0).rgb * 0.00472;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -8), 0).rgb * 0.00472;
	res += texelFetch(image, c + ivec2(0, -7), 0).rgb * 0.010233;
	res += texelFetch(image, c + ivec2(0, -6), 0).rgb * 0.020009;
	res += texelFetch(image, c + ivec2(0, -5), 0).rgb * 0.035289;
	res += texelFetch(image, c + ivec2(0, -4), 0).rgb * 0.056137;
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.080549;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.104247;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.121694;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.121694;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.104247;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.080549;
	res += texelFetch(image, c + ivec2(0, +4), 0).rgb * 0.056137;
	res += texelFetch(image, c + ivec2(0, +5), 0).rgb * 0.035289;
	res += texelFetch(image, c + ivec2(0, +6), 0).rgb * 0.020009;
	res += texelFetch(image, c + ivec2(0, +7), 0).rgb * 0.010233;
	res += texelFetch(image, c + ivec2(0, +8), 0).rgb * 0.00472;
#endif
#endif

#ifdef R9
	vec3 res = texelFetch(image, c, 0).rgb * 0.113597;
#ifdef H
	res += texelFetch(image, c + ivec2(-9, 0), 0).rgb * 0.004258;
	res += texelFetch(image, c + ivec2(-8, 0), 0).rgb * 0.008483;
	res += texelFetch(image, c + ivec2(-7, 0), 0).rgb * 0.015583;
	res += texelFetch(image, c + ivec2(-6, 0), 0).rgb * 0.026396;
	res += texelFetch(image, c + ivec2(-5, 0), 0).rgb * 0.04123;
	res += texelFetch(image, c + ivec2(-4, 0), 0).rgb * 0.059384;
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.07887;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.096592;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.109084;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.109084;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.096592;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.07887;
	res += texelFetch(image, c + ivec2(+4, 0), 0).rgb * 0.059384;
	res += texelFetch(image, c + ivec2(+5, 0), 0).rgb * 0.04123;
	res += texelFetch(image, c + ivec2(+6, 0), 0).rgb * 0.026396;
	res += texelFetch(image, c + ivec2(+7, 0), 0).rgb * 0.010233;
	res += texelFetch(image, c + ivec2(+8, 0), 0).rgb * 0.008483;
	res += texelFetch(image, c + ivec2(+9, 0), 0).rgb * 0.004258;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -9), 0).rgb * 0.004258;
	res += texelFetch(image, c + ivec2(0, -8), 0).rgb * 0.008483;
	res += texelFetch(image, c + ivec2(0, -7), 0).rgb * 0.010233;
	res += texelFetch(image, c + ivec2(0, -6), 0).rgb * 0.026396;
	res += texelFetch(image, c + ivec2(0, -5), 0).rgb * 0.04123;
	res += texelFetch(image, c + ivec2(0, -4), 0).rgb * 0.059384;
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.07887;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.096592;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.109084;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.109084;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.096592;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.07887;
	res += texelFetch(image, c + ivec2(0, +4), 0).rgb * 0.059384;
	res += texelFetch(image, c + ivec2(0, +5), 0).rgb * 0.04123;
	res += texelFetch(image, c + ivec2(0, +6), 0).rgb * 0.026396;
	res += texelFetch(image, c + ivec2(0, +7), 0).rgb * 0.015583;
	res += texelFetch(image, c + ivec2(0, +8), 0).rgb * 0.008483;
	res += texelFetch(image, c + ivec2(0, +9), 0).rgb * 0.004258;
#endif
#endif

#ifdef R10
	vec3 res = texelFetch(image, c, 0).rgb * 0.099476;
#ifdef H
	res += texelFetch(image, c + ivec2(-10, 0), 0).rgb * 0.004442;
	res += texelFetch(image, c + ivec2(-9, 0), 0).rgb * 0.008019;
	res += texelFetch(image, c + ivec2(-8, 0), 0).rgb * 0.013603;
	res += texelFetch(image, c + ivec2(-7, 0), 0).rgb * 0.021685;
	res += texelFetch(image, c + ivec2(-6, 0), 0).rgb * 0.032484;
	res += texelFetch(image, c + ivec2(-5, 0), 0).rgb * 0.045729;
	res += texelFetch(image, c + ivec2(-4, 0), 0).rgb * 0.060492;
	res += texelFetch(image, c + ivec2(-3, 0), 0).rgb * 0.075199;
	res += texelFetch(image, c + ivec2(-2, 0), 0).rgb * 0.087845;
	res += texelFetch(image, c + ivec2(-1, 0), 0).rgb * 0.096432;
	res += texelFetch(image, c + ivec2(+1, 0), 0).rgb * 0.096432;
	res += texelFetch(image, c + ivec2(+2, 0), 0).rgb * 0.087845;
	res += texelFetch(image, c + ivec2(+3, 0), 0).rgb * 0.075199;
	res += texelFetch(image, c + ivec2(+4, 0), 0).rgb * 0.060492;
	res += texelFetch(image, c + ivec2(+5, 0), 0).rgb * 0.045729;
	res += texelFetch(image, c + ivec2(+6, 0), 0).rgb * 0.032484;
	res += texelFetch(image, c + ivec2(+7, 0), 0).rgb * 0.021685;
	res += texelFetch(image, c + ivec2(+8, 0), 0).rgb * 0.013603;
	res += texelFetch(image, c + ivec2(+9, 0), 0).rgb * 0.008019;
	res += texelFetch(image, c + ivec2(+10, 0), 0).rgb * 0.004442;
#endif
#ifdef V
	res += texelFetch(image, c + ivec2(0, -10), 0).rgb * 0.004442;
	res += texelFetch(image, c + ivec2(0, -9), 0).rgb * 0.008019;
	res += texelFetch(image, c + ivec2(0, -8), 0).rgb * 0.013603;
	res += texelFetch(image, c + ivec2(0, -7), 0).rgb * 0.021685;
	res += texelFetch(image, c + ivec2(0, -6), 0).rgb * 0.032484;
	res += texelFetch(image, c + ivec2(0, -5), 0).rgb * 0.045729;
	res += texelFetch(image, c + ivec2(0, -4), 0).rgb * 0.060492;
	res += texelFetch(image, c + ivec2(0, -3), 0).rgb * 0.075199;
	res += texelFetch(image, c + ivec2(0, -2), 0).rgb * 0.087845;
	res += texelFetch(image, c + ivec2(0, -1), 0).rgb * 0.096432;
	res += texelFetch(image, c + ivec2(0, +1), 0).rgb * 0.096432;
	res += texelFetch(image, c + ivec2(0, +2), 0).rgb * 0.087845;
	res += texelFetch(image, c + ivec2(0, +3), 0).rgb * 0.075199;
	res += texelFetch(image, c + ivec2(0, +4), 0).rgb * 0.060492;
	res += texelFetch(image, c + ivec2(0, +5), 0).rgb * 0.045729;
	res += texelFetch(image, c + ivec2(0, +6), 0).rgb * 0.032484;
	res += texelFetch(image, c + ivec2(0, +7), 0).rgb * 0.021685;
	res += texelFetch(image, c + ivec2(0, +8), 0).rgb * 0.013603;
	res += texelFetch(image, c + ivec2(0, +9), 0).rgb * 0.008019;
	res += texelFetch(image, c + ivec2(0, +10), 0).rgb * 0.004442;
#endif
#endif

	o_color = vec4(res, 1.0);
}
