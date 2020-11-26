#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	std::string type = args[1];
	auto size = stou2(args[2]);
	auto b = Bitmap::create(size.x, size.y, 4);
	auto data = b->get_data();
	auto pitch = b->get_pitch();
	std::wstring output;

	if (type == "plain")
	{
		auto color = stoc4(args[3]);
		b = Bitmap::create(size.x, size.y, 4);
		for (auto i = 0; i < size.y; i++)
		{
			for (auto j = 0; j < size.x; j++)
				memcpy(data + i * pitch + j * 4, &color, sizeof(cvec4));
		}
		output = s2w(args[4]);
	}
	else if (type == "hori_stripes")
	{
		auto offset = std::stoul(args[3]);
		auto line_width = std::stoul(args[4]);
		auto spacing = std::stoul(args[5]);
		auto foreground_color = stoc4(args[6]);
		auto background_color = stoc4(args[7]);
		for (auto i = 0; i < size.y; i++)
		{
			for (auto j = 0; j < size.x; j++)
			{
				if ((i + offset) % spacing < line_width)
					memcpy(data + i * pitch + j * 4, &foreground_color, sizeof(cvec4));
				else
					memcpy(data + i * pitch + j * 4, &background_color, sizeof(cvec4));
			}
		}
		output = s2w(args[8]);
	}
	else if (type == "vert_stripes")
	{
		auto offset = std::stoul(args[3]);
		auto line_width = std::stoul(args[4]);
		auto spacing = std::stoul(args[5]);
		auto foreground_color = stoc4(args[6]);
		auto background_color = stoc4(args[7]);
		for (auto i = 0; i < size.y; i++)
		{
			for (auto j = 0; j < size.x; j++)
			{
				if ((j + offset) % spacing < line_width)
					memcpy(data + i * pitch + j * 4, &foreground_color, sizeof(cvec4));
				else
					memcpy(data + i * pitch + j * 4, &background_color, sizeof(cvec4));
			}
		}
		output = s2w(args[8]);
	}
	else if (type == "brick_wall")
	{
		auto offset = stou2(args[3]);
		auto line_width = std::stoul(args[4]);
		auto half_brick_width = std::stoul(args[5]);
		auto brick_height = std::stoul(args[6]);
		auto foreground_color = stoc4(args[7]);
		auto background_color = stoc4(args[8]);
		for (auto i = 0; i < size.y; i++)
		{
			for (auto j = 0; j < size.x; j++)
			{
				if ((i + offset.y) % brick_height < line_width)
					memcpy(data + i * pitch + j * 4, &foreground_color, sizeof(cvec4));
				else
					memcpy(data + i * pitch + j * 4, &background_color, sizeof(cvec4));
			}
		}
		for (auto i = 0; i < size.y; i++)
		{
			for (auto j = 0; j < size.x; j++)
			{
				auto ii = i + offset.x;
				auto jj = j + offset.y;
				if ((ii / brick_height) % 2 == (jj / half_brick_width) % 2 && ii % brick_height >= line_width && jj % half_brick_width < line_width)
					memcpy(data + i * pitch + j * 4, &foreground_color, sizeof(cvec4));
			}
		}
		output = s2w(args[9]);
	}

	b->save_to_file(output.c_str());
	b->release();

	return 0;
}
