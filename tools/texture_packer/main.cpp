#include <flame/serialize.h>
#include <flame/graphics/image.h>

using namespace flame;

int main(int argc, char **args)
{
	auto current_path = std::filesystem::current_path();
	graphics::ImageAtlas::generate(current_path);

	return 0;
}
