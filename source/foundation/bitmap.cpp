#include <flame/foundation/bitmap.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace flame
{
	void Bitmap::add_alpha_channel()
	{
		assert(channel == 3);

		auto new_data = new uchar[size.x() * size.y() * 4];
		auto dst = new_data;
		for (auto j = 0; j < size.y(); j++)
		{
			auto src = data + j * pitch;
			for (auto i = 0; i < size.x(); i++)
			{
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = 255;
			}
		}
		channel = 4;
		bpp = 32;
		pitch = size.x() * 4;
		data_size = pitch * size.y();
		delete[]data;
		data = new_data;
	}

	void Bitmap::swap_channel(int ch1, int ch2)
	{
		for (auto j = 0; j < size.y(); j++)
		{
			auto line = data + j * pitch;
			for (auto i = 0; i < size.x(); i++)
			{
				auto p = line + i * channel;
				std::swap(p[ch1], p[ch2]);
			}
		}
	}

	void Bitmap::copy_to(Bitmap* b, const Vec2u& src_off, const Vec2u& cpy_size, const Vec2u& dst_off)
	{
		assert(channel == b->channel);
		assert(src_off + cpy_size < size);

		for (auto j = 0; j < cpy_size.y(); j++)
		{
			auto src_line = data + (src_off.y() + j) * pitch;
			auto dst_line = b->data + (dst_off.y() + j) * b->pitch;
			memcpy(dst_line, src_line, cpy_size.x() * channel);
		}
	}

	void Bitmap::save(const std::wstring& wfilename)
	{
		auto filename = w2s(wfilename);
		auto ext = std::filesystem::path(filename).extension();

		if (ext == ".png")
			stbi_write_png(filename.c_str(), size.x(), size.y(), channel, data, pitch);
		else if (ext == ".bmp")
			stbi_write_bmp(filename.c_str(), size.x(), size.y(), channel, data);
	}

	Bitmap* Bitmap::create(const Vec2u& size, int channel, int bpp, unsigned char* data, bool data_owner)
	{
		auto b = new Bitmap;
		b->size = size;
		b->channel = channel;
		b->bpp = bpp;
		b->sRGB = false;
		b->calc_pitch();
		b->data_size = b->pitch * b->size.y();
		if (!data)
		{
			b->data = new unsigned char[b->data_size];
			memset(b->data, 0, b->data_size);
		}
		else
		{
			if (data_owner)
				b->data = data;
			else
			{
				b->data = new unsigned char[b->data_size];
				memcpy(b->data, data, b->data_size);
			}
		}
		return b;
	}

	Bitmap* Bitmap::create_from_file(const std::wstring& filename)
	{
		auto file = _wfopen(filename.c_str(), L"rb");
		if (!file)
			return nullptr;

		int cx, cy, channel;
		auto data = stbi_load_from_file(file, &cx, &cy, &channel, 4);
		if (!data)
			return nullptr;
		auto b = new Bitmap;
		b->sRGB = false;
		b->size.x() = cx;
		b->size.y() = cy;
		b->channel = 4;
		b->bpp = 32;
		b->calc_pitch();
		b->data_size = b->pitch * b->size.y();
		b->data = new unsigned char[b->data_size];
		memcpy(b->data, data, b->data_size);
		stbi_image_free(data);
		fclose(file);
		return b;
	}

	Bitmap* Bitmap::create_from_gif(const std::wstring& filename)
	{
		auto file = get_file_content(filename);

		int cx, cy, cz, channel;
		auto data = stbi_load_gif_from_memory((uchar*)file.first.get(), file.second, nullptr, &cx, &cy, &cz, &channel, 4);
		stbi_image_free(data);

		return nullptr;
	}

	void Bitmap::destroy(Bitmap* b)
	{
		delete[]b->data;
		delete b;
	}

	Bitmap* texture_bin_pack(const std::vector<Bitmap*>& textures)
	{
		struct Element
		{
			Vec2i pos;
			Bitmap* b;
		};
		std::vector<Element> elements;
		for (auto& t : textures)
		{
			Element e;
			e.pos = Vec2u(-1);
			e.b = t;
			elements.push_back(e);
		}
		std::sort(elements.begin(), elements.end(), [](const Element& a, const Element& b) {
			return max(a.b->size.x(), a.b->size.y()) > max(b.b->size.x(), b.b->size.y());
		});

		auto w = 512, h = 512;
		struct Node
		{
			Vec2u size;
			std::unique_ptr<Node> right;
			std::unique_ptr<Node> bottom;
		};
		auto tree = new Node;
		tree->size.x() = w;
		tree->size.y() = h;

		for (auto& e : elements)
		{

		}

		return nullptr;
	}
}
