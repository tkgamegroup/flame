#include <flame/foundation/foundation.h>

using namespace flame;



int main(int argc, char** args) 
{
	auto f = 1000.f;
	auto n = 0.1f;
	auto z = -6.7f;

	mat4 Result(0.f);
	Result[2][2] = f / (n - f);
	Result[2][3] = -1;
	Result[3][2] = -(f * n) / (f - n);

	auto wtf1 = Result * vec4(0, 0, z, 1);
	wtf1 /= wtf1.w;

	auto wtf = (f / (n - f)) * z - f * n / (f - n);
	wtf = wtf / -z;

	return 0;
}

