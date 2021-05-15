#include <flame/foundation/foundation.h>
#include <flame/graphics/model.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	std::filesystem::path input;
	if (argc == 2)
		input = args[1];

	goto process;

process:

	Model::convert(input.c_str());
	printf("converted\n");
	return 0;
}
