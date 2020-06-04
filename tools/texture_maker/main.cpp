#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	Bitmap* b = nullptr;
	std::wstring output;

	std::string type = args[1];
	if (type == "plain")
	{
		auto size = stou2(args[2]);
		auto color = stoc4(args[3]);
		b = Bitmap::create(size, 4, 32);
		for (auto i = 0; i < size.y(); i++)
		{
			for (auto j = 0; j < size.x(); j++)
				memcpy(b->data + i * b->pitch + j * 4, &color, sizeof(Vec4c));
		}
		output = s2w(args[4]);
	}
	else if (type == "hori_stripes")
	{
		auto size = stou2(args[2]);
		auto offset = std::stoul(args[3]);
		auto line_width = std::stoul(args[4]);
		auto spacing = std::stoul(args[5]);
		auto foreground_color = stoc4(args[6]);
		auto background_color = stoc4(args[7]);
		b = Bitmap::create(size, 4, 32);
		for (auto i = 0; i < size.y(); i++)
		{
			for (auto j = 0; j < size.x(); j++)
			{
				if ((i + offset) % spacing < line_width)
					memcpy(b->data + i * b->pitch + j * 4, &foreground_color, sizeof(Vec4c));
				else
					memcpy(b->data + i * b->pitch + j * 4, &background_color, sizeof(Vec4c));
			}
		}
		output = s2w(args[8]);
	}
	else if (type == "vert_stripes")
	{
		auto size = stou2(args[2]);
		auto offset = std::stoul(args[3]);
		auto line_width = std::stoul(args[4]);
		auto spacing = std::stoul(args[5]);
		auto foreground_color = stoc4(args[6]);
		auto background_color = stoc4(args[7]);
		b = Bitmap::create(size, 4, 32);
		for (auto i = 0; i < size.y(); i++)
		{
			for (auto j = 0; j < size.x(); j++)
			{
				if ((j + offset) % spacing < line_width)
					memcpy(b->data + i * b->pitch + j * 4, &foreground_color, sizeof(Vec4c));
				else
					memcpy(b->data + i * b->pitch + j * 4, &background_color, sizeof(Vec4c));
			}
		}
		output = s2w(args[8]);
	}
	else if (type == "brick_wall")
	{
		auto size = stou2(args[2]);
		auto offset = stou2(args[3]);
		auto line_width = std::stoul(args[4]);
		auto half_brick_width = std::stoul(args[5]);
		auto brick_height = std::stoul(args[6]);
		auto foreground_color = stoc4(args[7]);
		auto background_color = stoc4(args[8]);
		b = Bitmap::create(size, 4, 32);
		for (auto i = 0; i < size.y(); i++)
		{
			for (auto j = 0; j < size.x(); j++)
			{
				if ((i + offset.y()) % brick_height < line_width)
					memcpy(b->data + i * b->pitch + j * 4, &foreground_color, sizeof(Vec4c));
				else
					memcpy(b->data + i * b->pitch + j * 4, &background_color, sizeof(Vec4c));
			}
		}
		for (auto i = 0; i < size.y(); i++)
		{
			for (auto j = 0; j < size.x(); j++)
			{
				auto ii = i + offset.x();
				auto jj = j + offset.y();
				if ((ii / brick_height) % 2 == (jj / half_brick_width) % 2 && ii % brick_height >= line_width && jj % half_brick_width < line_width)
					memcpy(b->data + i * b->pitch + j * 4, &foreground_color, sizeof(Vec4c));
			}
		}
		output = s2w(args[9]);
	}

	if (b)
		Bitmap::save_to_file(b, output.c_str());

	return 0;
}
