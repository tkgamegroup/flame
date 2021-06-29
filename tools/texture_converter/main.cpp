#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	std::string input;
	std::string output;
	auto srgb = false;
	auto ap = pack_args(argc, args);
	if (!ap.get_item("-i", input) || !ap.get_item("-o", output))
		goto show_usage;
	srgb = ap.has("-srgb");

	goto process;

show_usage:
	printf("usage: model_converter -i filename -o filename\n"
		"-i: specify the input texture path\n"
		"-o: specify the output texture path\n");
	return 0;

process:
	auto d = Device::create(true);
	auto i = Image::get(d, std::filesystem::path(input).c_str(), srgb);
	i->save(std::filesystem::path(output).c_str());

	printf("converted\n");

	return 0;
}
