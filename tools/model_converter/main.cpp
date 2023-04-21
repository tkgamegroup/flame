#include <flame/foundation/foundation.h>
#include <flame/graphics/model.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	std::filesystem::path input;
	vec3 scaling = vec3(1.f);
	if (argc >= 2)
	{
		input = args[1];

		for (auto i = 2; i < argc; i++)
		{
			if (std::string(args[i]) == "-scaling")
			{
				scaling = s2t<3, float>(args[i + 1]);
				i++;
			}
		}

		Model::convert(input, vec3(0.f), scaling);
		printf("converted\n");
	}
	return 0;
}
