#include <flame/foundation/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	Bitmap* b = nullptr;
	std::wstring output;

	std::string type = args[1];
	if (type == "plain")
	{
		b = Bitmap::create_with_plaincolor(stou2(args[2]), stoc4(args[3]));
		output = s2w(args[4]);
	}
	else if (type == "hori_stripes")
	{
		b = Bitmap::create_with_horizontalstripes_pattern(stou2(args[2]), std::stoul(args[3]), std::stoul(args[4]), std::stoul(args[5]), stoc4(args[6]), stoc4(args[7]));
		output = s2w(args[8]);
	}
	else if (type == "vert_stripes")
	{
		b = Bitmap::create_with_verticalstripes_pattern(stou2(args[2]), std::stoul(args[3]), std::stoul(args[4]), std::stoul(args[5]), stoc4(args[6]), stoc4(args[7]));
		output = s2w(args[8]);
	}
	else if (type == "brick_wall")
	{
		b = Bitmap::create_with_brickwall_pattern(stou2(args[2]), stou2(args[3]), std::stoul(args[4]), std::stoul(args[5]), std::stoul(args[6]), stoc4(args[7]), stoc4(args[8]));
		output = s2w(args[9]);
	}

	if (b)
		Bitmap::save_to_file(b, output);

	return 0;
}
