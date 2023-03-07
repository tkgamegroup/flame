#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

int main(int argc, char** args) 
{
	auto q0 = angleAxis(radians(45.f), vec3(0.f, 1.f, 0.f));
	auto q1 = angleAxis(radians(30.f), vec3(1.f, 0.f, 0.f));
	auto diff = inverse(q0) * q1;
	auto wtf = q0 * diff;
	return 0;
}

