#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/shader.h>

using namespace flame;
using namespace graphics;

int main(int argc, char **args)
{
	if (argc == 2)
	{
		auto filename = std::filesystem::path(args[1]);
		wprintf(L"compile: %s\n", filename.c_str());

		auto ext = filename.extension();
		if (ext == L".dsl")
			DescriptorSetLayout::get(nullptr, filename.c_str());
		else if (ext == L".pll")
			PipelineLayout::get(nullptr, filename.c_str());
		else if (ext == L".pl")
			Pipeline::get(nullptr, filename.c_str());
		else
			Shader::get(nullptr, filename.c_str(), "", "");
	}
	else
		printf("usage: shader_compiler xxx\n");

	return 0;
}
