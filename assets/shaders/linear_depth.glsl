float linear_depth(float z, float n, float f)
{
	return n / (f + z * (n - f));
}
