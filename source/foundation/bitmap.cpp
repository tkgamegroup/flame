#include <flame/foundation/bitmap.h>
#include <flame/foundation/serialize.h>

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
		assert(src_off + cpy_size <= size);
		assert(_dst_off + cpy_size <= b->size + (border ? 2U : 0U));

		auto dst_off = _dst_off + (border ? 1U : 0U);
		for (auto i = 0; i < cpy_size.y(); i++)
		{
			auto src_line = data + (src_off.y() + i) * pitch + src_off.x() * channel;
			auto dst_line = b->data + (dst_off.y() + i) * b->pitch + dst_off.x() * channel;
			memcpy(dst_line, src_line, cpy_size.x() * channel);
		}

		if (border)
		{
			memcpy(b->data + (dst_off.y() - 1) * b->pitch + dst_off.x() * channel, data + src_off.y() * pitch + src_off.x() * channel, cpy_size.x() * channel); // top line
			memcpy(b->data + (dst_off.y() + cpy_size.y()) * b->pitch + dst_off.x() * channel, data + (src_off.y() + cpy_size.y() - 1) * pitch + src_off.x() * channel, cpy_size.x() * channel); // bottom line
			for (auto i = 0; i < cpy_size.y(); i++)
				memcpy(b->data + (dst_off.y() + i) * b->pitch + (dst_off.x() - 1) * channel, data + (src_off.y() + i) * pitch + src_off.x() * channel, channel); // left line
			for (auto i = 0; i < cpy_size.y(); i++)
				memcpy(b->data + (dst_off.y() + i) * b->pitch + (dst_off.x() + cpy_size.x()) * channel, data + (src_off.y() + i) * pitch + (src_off.x() + cpy_size.x() - 1) * channel, channel); // left line

			memcpy(b->data + (dst_off.y() - 1) * b->pitch + (dst_off.x() - 1) * channel, data + src_off.y() * pitch + src_off.x() * channel, channel); // left top corner
			memcpy(b->data + (dst_off.y() - 1) * b->pitch + (dst_off.x() + cpy_size.x()) * channel, data + src_off.y() * pitch + (src_off.x() + cpy_size.x() - 1) * channel, channel); // right top corner
			memcpy(b->data + (dst_off.y() + cpy_size.y()) * b->pitch + (dst_off.x() - 1) * channel, data + (src_off.y() + cpy_size.y() - 1) * pitch + src_off.x() * channel, channel); // left bottom corner
			memcpy(b->data + (dst_off.y() + cpy_size.y()) * b->pitch + (dst_off.x() + cpy_size.x()) * channel, data + (src_off.y() + cpy_size.y() - 1) * pitch + (src_off.x() + cpy_size.x() - 1) * channel, channel); // right bottom corner
		}
	}

	Bitmap* Bitmap::create(const Vec2u& size, uint channel, uint bpp, uchar* data, bool move)
	{
		auto b = new Bitmap;
		b->size = size;
		b->channel = channel;
		b->bpp = bpp;
		b->pitch = get_pitch(b->size.x() * bpp / 8);
		b->data_size = b->pitch * b->size.y();
		if (move)
			b->data = data;
		else
		{
			b->data = new uchar[b->data_size];
			if (!data)
				memset(b->data, 0, b->data_size);
			else
			{
				if (data != FLAME_INVALID_POINTER)
					memcpy(b->data, data, b->data_size);
			}
		}
		b->srgb = false;
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
		auto b = Bitmap::create(Vec2u(cx, cy), 4, 32, data);
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

	struct AtlasPrivate : Atlas
	{
		std::vector<Piece> pieces;
	};

	const std::vector<Atlas::Piece>& Atlas::pieces() const
	{
		return ((AtlasPrivate*)this)->pieces;
	}

	struct BinPackNode
	{
		bool used;
		Vec2u pos;
		Vec2u size;
		std::unique_ptr<BinPackNode> right;
		std::unique_ptr<BinPackNode> bottom;
	};

	BinPackNode* bin_pack_find_node(BinPackNode* n, const Vec2u& size)
	{
		if (!n->used && n->size >= size)
		{
			n->used = true;
			n->right.reset(new BinPackNode);
			n->right->used = false;
			n->right->pos = n->pos + Vec2u(size.x(), 0);
			n->right->size = Vec2u(n->size.x() - size.x(), size.y());
			n->bottom.reset(new BinPackNode);
			n->bottom->used = false;
			n->bottom->pos = n->pos + Vec2u(0, size.y());
			n->bottom->size = Vec2u(n->size.x(), n->size.x() - size.y());
			return n;
		}
		if (!n->right || !n->bottom)
			return nullptr;
		auto n1 = bin_pack_find_node(n->right.get(), size);
		if (n1)
			return n1;
		auto n2 = bin_pack_find_node(n->bottom.get(), size);
		if (n2)
			return n2;
		return nullptr;
	}

	void Atlas::bin_pack(const std::vector<std::wstring>& inputs, const std::wstring& output, bool border)
	{
		struct Piece
		{
			std::string filename;
			Bitmap* b;
			Vec2i pos;
		};
		std::vector<Piece> pieces;
		auto output_dir = std::filesystem::path(output).parent_path();
		for (auto& i : inputs)
		{
			Piece p;
			p.filename = std::filesystem::path(i).lexically_relative(output_dir).make_preferred().string();
			p.pos = Vec2i(-1);
			pieces.push_back(p);
		}
		std::sort(pieces.begin(), pieces.end(), [](const Piece& a, const Piece& b) {
			return max(a.b->size.x(), a.b->size.y()) > max(b.b->size.x(), b.b->size.y());
		});

		auto w = 512, h = 512;
		auto tree = std::make_unique<BinPackNode>();
		tree->used = false;
		tree->pos = Vec2u(0);
		tree->size.x() = w;
		tree->size.y() = h;

		for (auto& p : pieces)
		{
			auto n = bin_pack_find_node(tree.get(), p.b->size + Vec2i(border ? 2 : 0));
			if (n)
				p.pos = n->pos;
		}

		auto b = Bitmap::create(Vec2u(w, h), 4, 32);
		for (auto& e : pieces)
		{
			if (e.pos >= 0)
				e.b->copy_to(b, Vec2u(0), e.b->size, Vec2u(e.pos), border);
		}

		Bitmap::save_to_file(b, output);
		std::ofstream atlas_file(output + L".atlas");
		for (auto& p : pieces)
		{
			atlas_file << p.filename + " " + to_string(Vec4u(Vec2u(p.pos) + (border ? 1U : 0U), p.b->size)) + "\n";
			Bitmap::destroy(p.b);
		}
		atlas_file.close();
	}

	Atlas* Atlas::load(const std::wstring& filename)
	{
		std::ifstream file(filename);
		if (!file.good())
			return nullptr;

		auto atlas = new AtlasPrivate;

		while (!file.eof())
		{
			Piece piece;

			std::string line;
			std::getline(file, line);
			if (line.empty())
				break;
			std::stringstream ss(line);
			ss >> piece.filename;
			std::string t;
			ss >> t;
			auto v = stou4(t.c_str());
			piece.pos = Vec2i(v.x(), v.y());
			piece.size = Vec2i(v.z(), v.w());

			atlas->pieces.push_back(piece);
		}

		file.close();

		return atlas;
	}
}
