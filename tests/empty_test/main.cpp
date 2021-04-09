#include <flame/foundation/typeinfo.h>

using namespace flame;

const std::vector<int*> a;

int main(int argc, char** args)
{
	a.data();
	return 0;
}
