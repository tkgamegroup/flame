#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/shader.h>

using namespace flame;
using namespace graphics;

int main(int argc, char **args)
{
	if (argc == 2)
	{
		auto device = Device::create(true);

		auto filename = std::filesystem::path(args[1]);
		auto ext = filename.extension();
		if (ext == L".dsl")
			DescriptorSetLayout::get(device, filename.c_str());
		else if (ext == L".pll")
			PipelineLayout::get(device, filename.c_str());
		else if (ext == L".pl")
			Pipeline::get(device, filename.c_str());
		else
			Shader::get(device, filename.c_str(), "", "");
	}
	else
		printf("usage: shader_compiler xxx\n");

	return 0;
}
