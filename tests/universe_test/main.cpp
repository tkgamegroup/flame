#include <flame/universe/world.h>
#include <flame/universe/entity.h>

using namespace flame;

int main(int argc, char** args)
{
	get_looper()->loop([](Capture&) {
	}, Capture());

	return 0;
}
