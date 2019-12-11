#include <flame/foundation/blueprint.h>

using namespace flame;

int main(int argc, char **args)
{
	auto bp = BP::create_from_file(L"bp");
	BP::destroy(bp);

	system("pause");

	return 0;
}
