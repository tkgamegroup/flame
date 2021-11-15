#pragma once

#include "foundation.h"

namespace flame
{
	FLAME_FOUNDATION_TYPE(Bitmap);

	struct Bitmap
	{
		uvec2 size;
		uint chs;
		// bits per pixel
		uint bpp;
		uint pitch;
		uchar* data;
		uint data_size;
		bool srgb;

		virtual ~Bitmap() {}
		virtual void change_format(uint chs) = 0;
		virtual void swap_channel(uint ch1, uint ch2) = 0;
		virtual void copy_to(BitmapPtr dst, const uvec2& size, const ivec2& src_off, const ivec2& dst_off, bool border = false) = 0;
		virtual void srgb_to_linear() = 0;
		virtual void save(const std::filesystem::path& filename) = 0;

		FLAME_FOUNDATION_EXPORTS static Bitmap* create(const uvec2& size, uint chs = 4, uint bpp = 32, uchar* data = nullptr);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create(const std::filesystem::path& filename);
	};

	struct BinPackNode
	{
		bool used = false;
		uvec2 pos = uvec2(0);
		uvec2 size;
		std::unique_ptr<BinPackNode> right;
		std::unique_ptr<BinPackNode> bottom;

		BinPackNode(const uvec2& size) :
			size(size)
		{
		}

		BinPackNode* find(const uvec2& s)
		{
			if (!used && size.x >= s.x && size.y >= size.y)
			{
				used = true;
				right.reset(new BinPackNode(uvec2(size.x - s.x, s.y)));
				right->pos = pos + uvec2(s.x, 0);
				bottom.reset(new BinPackNode(uvec2(size.x, size.y - s.y)));
				bottom->pos = pos + uvec2(0, s.y);
				return this;
			}
			if (!right || !bottom)
				return nullptr;
			auto n1 = right->find(s);
			if (n1)
				return n1;
			auto n2 = bottom->find(s);
			if (n2)
				return n2;
			return nullptr;
		}
	};
}
