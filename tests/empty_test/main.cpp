#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

using namespace flame;

void f(std::optional<int> a)
{
	auto b = a ? a : 2;
	printf("%d\n", b.value());
}

int main(int argc, char** args) 
{
	f(5);
	f({});

	return 0;
}

