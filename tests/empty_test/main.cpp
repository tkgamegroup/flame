#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

int main(int argc, char** args) 
{
	auto q = quat(1, 0, 0, 0);
	auto mat = mat3(q);
	q = q * angleAxis(radians(90.f), vec3(0, 1, 0));
	mat = mat3(q);
	auto axis = q * vec3(1, 0, 0);
	q = angleAxis(radians(90.f), axis) * q;
	mat = mat3(q);

	return 0;
}

