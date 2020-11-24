const float PI = 3.14159265359;
const float PI_INV = 1.0 / PI;

float ramp2(float v, float c1, float p1, float c2, float p2)
{
	return mix(c1, mix(mix(c1, c2, (v - p1) / (p2 - p1)), c2, step(p2, v)), step(p1, v));
}

vec2 panorama(vec3 v)
{
	return vec2(1.0 + atan(v.x, v.z) * PI_INV, acos(v.y) * PI_INV);
}
