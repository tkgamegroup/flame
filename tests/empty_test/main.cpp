#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

int main(int argc, char** args) 
{
	mat3x2 m1;
	m1[0] = vec2(1, 0);
	m1[1] = vec2(0, 1);
	m1[2] = vec2(5, 10);
	auto wtf = m1 * vec3(5, 10, 1);

	mat3x2 m2;
	m2[0] = vec2(2, 0);
	m2[1] = vec2(0, 3);
	m2[2] = vec2(5, 10);

	auto wtf2 = mat3x2(mat3(m1) * mat3(m2));
	auto wtf3 = wtf2 * vec3(5, 10, 1);

	return 0;
}

