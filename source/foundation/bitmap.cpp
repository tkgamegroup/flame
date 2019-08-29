#include <flame/foundation/bitmap.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <functional>

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

	void Bitmap::swap_channel(uint ch1, uint ch2)
	{
		assert(bpp / channel == 8);

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

	void Bitmap::copy_to(Bitmap* b, const Vec2u& src_off, const Vec2u& cpy_size, const Vec2u& _dst_off, bool border)
	{
		assert(bpp / channel == 8);
		assert(channel == b->channel && bpp == b->bpp);
		assert(src_off + cpy_size < size + (border ? 2U : 0U));

		auto dst_off = _dst_off + (border ? 1U : 0U);
		for (auto i = 0; i < cpy_size.y(); i++)
		{
			auto src_line = data + (src_off.y() + i) * pitch + src_off.x() * channel;
			auto dst_line = b->data + (dst_off.y() + i) * pitch + dst_off.x() * channel;
			memcpy(dst_line, src_line, cpy_size.x() * channel);
		}

		if (border)
		{
			memcpy(b->data + (dst_off.y() - 1) * pitch + dst_off.x() * channel, data + src_off.y() * pitch + src_off.x() * channel, cpy_size.x() * channel); // top line
			memcpy(b->data + (dst_off.y() + cpy_size.y()) * pitch + dst_off.x() * channel, data + (src_off.y() + cpy_size.y() - 1) * pitch + src_off.x() * channel, cpy_size.x() * channel); // bottom line
			for (auto i = 0; i < cpy_size.y(); i++)
				memcpy(b->data + (dst_off.y() + i) * pitch + (dst_off.x() - 1) * channel, data + (src_off.y() + i) * pitch + src_off.x() * channel, channel); // left line
			for (auto i = 0; i < cpy_size.y(); i++)
				memcpy(b->data + (dst_off.y() + i) * pitch + (dst_off.x() + cpy_size.x()) * channel, data + (src_off.y() + i) * pitch + (src_off.x() + cpy_size.x() - 1) * channel, channel); // left line

			memcpy(b->data + (dst_off.y() - 1) * pitch + (dst_off.x() - 1) * channel, data + src_off.y() * pitch + src_off.x() * channel, channel); // left top corner
			memcpy(b->data + (dst_off.y() - 1) * pitch + (dst_off.x() + cpy_size.x()) * channel, data + src_off.y() * pitch + (src_off.x() + cpy_size.x() - 1) * channel, channel); // right top corner
			memcpy(b->data + (dst_off.y() + cpy_size.y()) * pitch + (dst_off.x() - 1) * channel, data + (src_off.y() + cpy_size.y() - 1) * pitch + src_off.x() * channel, channel); // left bottom corner
			memcpy(b->data + (dst_off.y() + cpy_size.y()) * pitch + (dst_off.x() + cpy_size.x()) * channel, data + (src_off.y() + cpy_size.y() - 1) * pitch + (src_off.x() + cpy_size.x() - 1) * channel, channel); // right bottom corner
		}
	}

	Bitmap* Bitmap::create(const Vec2u& size, uint channel, uint bpp, uchar* data)
	{
		auto b = new Bitmap;
		b->size = size;
		b->channel = channel;
		b->bpp = bpp;
		b->srgb = false;
		b->calc_pitch();
		b->data_size = b->pitch * b->size.y();
		b->data = new uchar[b->data_size];
		if (!data)
			memset(b->data, 0, b->data_size);
		else
			memcpy(b->data, data, b->data_size);
		return b;
	}

	Bitmap* Bitmap::create_with_plaincolor(const Vec2u& size, const Vec4c& color)
	{
		auto b = new Bitmap;
		b->size = size;
		b->channel = 4;
		b->bpp = 32;
		b->srgb = false;
		b->calc_pitch();
		b->data_size = b->pitch * b->size.y();
		b->data = new uchar[b->data_size];
		for (auto i = 0; i < size.y(); i++)
		{
			for (auto j = 0; j < size.x(); j++)
				memcpy(b->data + i * b->pitch + j * 4, &color, sizeof(Vec4c));
		}
		return b;
	}

	Bitmap* Bitmap::create_with_horizontalstripes_pattern(const Vec2u& size, uint offset, uint line_width, uint spacing, const Vec4c& foreground_color, const Vec4c& background_color)
	{
		auto b = new Bitmap;
		b->size = size;
		b->channel = 4;
		b->bpp = 32;
		b->srgb = false;
		b->calc_pitch();
		b->data_size = b->pitch * b->size.y();
		b->data = new uchar[b->data_size];
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
		return b;
	}

	Bitmap* Bitmap::create_with_verticalstripes_pattern(const Vec2u& size, uint offset, uint line_width, uint spacing, const Vec4c& foreground_color, const Vec4c& background_color)
	{
		auto b = new Bitmap;
		b->size = size;
		b->channel = 4;
		b->bpp = 32;
		b->srgb = false;
		b->calc_pitch();
		b->data_size = b->pitch * b->size.y();
		b->data = new uchar[b->data_size];
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
		return b;
	}

	Bitmap* Bitmap::create_with_brickwall_pattern(const Vec2u& size, const Vec2u& offset, uint line_width, uint half_brick_width, uint brick_height, const Vec4c& foreground_color, const Vec4c& background_color)
	{
		auto b = new Bitmap;
		b->size = size;
		b->channel = 4;
		b->bpp = 32;
		b->srgb = false;
		b->calc_pitch();
		b->data_size = b->pitch * b->size.y();
		b->data = new uchar[b->data_size];
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
		b->srgb = false;
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

	void Bitmap::save_to_file(Bitmap* b, const std::wstring& filename)
	{
		auto ext = std::filesystem::path(filename).extension();

		if (ext == L".png")
			stbi_write_png(w2s(filename).c_str(), b->size.x(), b->size.y(), b->channel, b->data, b->pitch);
		else if (ext == L".bmp")
			stbi_write_bmp(w2s(filename).c_str(), b->size.x(), b->size.y(), b->channel, b->data);
	}

	void Bitmap::destroy(Bitmap* b)
	{
		delete[]b->data;
		delete b;
	}

	Mail<std::pair<Bitmap*, std::vector<std::pair<std::string, Vec4u>>>> bin_pack(const std::vector<std::pair<Bitmap*, std::string>>& textures)
	{
		auto channel = textures[0].first->channel;
		auto bpp = textures[0].first->bpp;

		struct Package
		{
			Vec2i pos;
			Bitmap* b;
			std::string id;
		};
		std::vector<Package> packages;
		for (auto& t : textures)
		{
			assert(t.first->channel == channel && t.first->bpp == bpp);

			Package p;
			p.pos = Vec2i(-1);
			p.b = t.first;
			p.id = t.second;
			packages.push_back(p);
		}
		std::sort(packages.begin(), packages.end(), [](const Package& a, const Package& b) {
			return max(a.b->size.x(), a.b->size.y()) > max(b.b->size.x(), b.b->size.y());
		});

		auto w = 512, h = 512;
		struct Node
		{
			bool used;
			Vec2u pos;
			Vec2u size;
			std::unique_ptr<Node> right;
			std::unique_ptr<Node> bottom;
		};
		auto tree = std::make_unique<Node>();
		tree->used = false;
		tree->pos = Vec2u(0);
		tree->size.x() = w;
		tree->size.y() = h;

		std::function<Node*(Node * n, const Vec2u & size)> find_node;
		find_node = [&](Node* n, const Vec2u& size)->Node* {
			if (!n->used && n->size >= size)
			{
				n->used = true;
				n->right.reset(new Node);
				n->right->used = false;
				n->right->pos = n->pos + Vec2u(size.x(), 0);
				n->right->size = Vec2u(n->size.x() - size.x(), size.y());
				n->bottom.reset(new Node);
				n->bottom->used = false;
				n->bottom->pos = n->pos + Vec2u(0, size.y());
				n->bottom->size = Vec2u(n->size.x(), n->size.x() - size.y());
				return n;
			}
			if (!n->right || !n->bottom)
				return nullptr;
			auto n1 = find_node(n->right.get(), size);
			if (n1)
				return n1;
			auto n2 = find_node(n->bottom.get(), size);
			if (n2)
				return n2;
			return nullptr;
		};

		for (auto& p : packages)
		{
			auto n = find_node(tree.get(), p.b->size + Vec2i(2));
			if (n)
				p.pos = n->pos;
		}

		auto b = Bitmap::create(Vec2u(w, h), channel, bpp);
		for (auto& e : packages)
		{
			if (e.pos >= 0)
				e.b->copy_to(b, Vec2u(0), e.b->size, Vec2u(e.pos), true);
		}

		auto ret = new_mail<std::pair<Bitmap*, std::vector<std::pair<std::string, Vec4u>>>>();
		ret.p->first = b;
		for (auto& p : packages)
			ret.p->second.emplace_back(p.id, Vec4u(Vec2u(p.pos), p.b->size));

		return ret;
	}
}
