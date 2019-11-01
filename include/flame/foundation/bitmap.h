#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Bitmap
	{
		Vec2u size;
		uint channel;
		uint bpp;
		uint pitch;
		uchar* data;
		uint data_size;
		bool srgb;

		FLAME_FOUNDATION_EXPORTS void add_alpha_channel();
		FLAME_FOUNDATION_EXPORTS void swap_channel(uint ch1, uint ch2);
		FLAME_FOUNDATION_EXPORTS void copy_to(Bitmap* b, const Vec2u& src_off, const Vec2u& cpy_size, const Vec2u& dst_off, bool border = false);

		FLAME_FOUNDATION_EXPORTS static Bitmap* create(const Vec2u& size, uint channel, uint bpp, uchar* data = nullptr, bool move = false);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_from_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_from_gif(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(Bitmap* b, const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(Bitmap* b);
	};

	struct Atlas
	{
		struct Piece
		{
			std::wstring filename;
			Vec2i pos;
			Vec2i size;
			Vec2f uv0;
			Vec2f uv1;
		};

		FLAME_FOUNDATION_EXPORTS Bitmap* bitmap() const;
		FLAME_FOUNDATION_EXPORTS const std::vector<Piece>& pieces() const;

		int find_piece(const std::wstring& filename) const
		{
			for (auto i = 0; i < pieces().size(); i++)
			{
				if (pieces()[i].filename == filename)
					return i;
			}
			return -1;
		}

		FLAME_FOUNDATION_EXPORTS static void pack(const std::vector<std::wstring>& inputs, const std::wstring& output, bool border);
		FLAME_FOUNDATION_EXPORTS static Atlas* load(const std::wstring& filename /* the image filename, not the atlas */);
		FLAME_FOUNDATION_EXPORTS static void destroy(Atlas* a);
	};

	struct BinPackNode
	{
		bool used;
		Vec2u pos;
		Vec2u size;
		std::unique_ptr<BinPackNode> right;
		std::unique_ptr<BinPackNode> bottom;

		BinPackNode(uint w, uint h)
		{
			used = false;
			size.x() = w;
			size.y() = h;
		}

		BinPackNode* find(const Vec2u& _size)
		{
			if (!used && size >= _size)
			{
				used = true;
				right.reset(new BinPackNode(size.x() - _size.x(), _size.y()));
				right->pos = pos + Vec2u(_size.x(), 0);
				bottom.reset(new BinPackNode(size.x(), size.y() - _size.y()));
				bottom->pos = pos + Vec2u(0, _size.y());
				return this;
			}
			if (!right || !bottom)
				return nullptr;
			auto n1 = right->find(size);
			if (n1)
				return n1;
			auto n2 = bottom->find(size);
			if (n2)
				return n2;
			return nullptr;
		}
	};
}
