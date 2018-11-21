const float PI = 3.1415926535;
const float PI_INV = 0.3183098861;

float linear_depth_ortho(float z, float depth_near, float depth_far)
{
	z = z * 0.5 + 0.5;
	return z * (depth_far - depth_near) + depth_near;
}

float linear_depth_perspective(float z, float depth_near, float depth_far)
{
	float a = (1.0 - depth_far / depth_near) * 0.5 / depth_far;
	float b = (1.0 + depth_far / depth_near) * 0.5 / depth_far;
	return 1.0 / (a * z + b);
}

vec2 panorama(vec3 v)
{
	return vec2(0.5 + 0.5 * atan(v.x, -v.z) * PI_INV, acos(v.y) * PI_INV);
}

vec3 inverse_panorama(vec2 tc)
{
	float y = cos(tc.y * PI);
	float oneMinusY2 = 1.0 - y * y;
	float alpha = (tc.x + 0.25) * 2 * PI;
	return normalize(vec3(cos(alpha) * oneMinusY2, y, sin(alpha) * oneMinusY2));
}

float rand2d(vec2 v)
{
	return fract(cos(v.x * (12.9898) + v.y * (4.1414)) * 43758.5453);
}

float noise2d(vec2 v)
{
	const uint SC = 250;

	v /= SC;
	vec2 vf = fract(v);
	vec2 vi = floor(v);

	float r0 = rand2d(vi);
	float r1 = rand2d(vi + vec2(1.0, 0.0));
	float r2 = rand2d(vi + vec2(0.0, 1.0));
	float r3 = rand2d(vi + vec2(1.0, 1.0));

	vec2 vs = 3.0 * vf * vf - 2.0 * vf * vf * vf;

	return mix(mix(r0, r1, vs.x),
			mix(r2, r3, vs.x),
			vs.y);
}

float fbm2d(vec2 v)
{
	float r = 0.0;

	float a = 1.0 / 3.0;
	for (int i = 0; i < 4; i++)
	{
		r += noise2d(v) * a;
		a /= 3.0;
		v *= 3.0;
	}

	return r;
}

const float esm_factor = 300.0;
