const float PI = 3.14159265359;
const float PI_INV = 1.0 / PI;

float ramp2(float v, float c1, float p1, float c2, float p2)
{
	return mix(c1, mix(mix(c1, c2, (v - p1) / (p2 - p1)), c2, step(p2, v)), step(p1, v));
}

vec2 panorama(vec3 v)
{
	return vec2(0.5 + 0.5 * atan(v.x, v.z) * PI_INV, acos(v.y) * PI_INV);
}

vec4 pack_uint_to_v4(uint id)
{
	vec4 ret;
	ret[0] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[1] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[2] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[3] = (id & 0xff) / 255.0;
	return ret;
}

float linear_depth(float near, float far, float d)
{
	return 2.0 * near * far / (far + near - d * (far - near));
}
