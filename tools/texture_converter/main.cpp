#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>

using namespace flame;
using namespace graphics;

int main(int argc, char** args)
{
	auto ap = parse_args(argc, args);
	auto input = ap.get_item("-i");
	auto output = ap.get_item("-o");
	if (input.empty() || output.empty())
		goto show_usage;
	auto srgb = ap.has("-srgb");

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
