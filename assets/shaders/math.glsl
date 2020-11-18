float ramp2(float v, float c1, float p1, float c2, float p2)
{
	return mix(c1, mix(mix(c1, c2, (v - p1) / (p2 - p1)), c2, step(p2, v)), step(p1, v));
}
