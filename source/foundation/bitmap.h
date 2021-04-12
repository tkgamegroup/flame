#pragma once

#include "foundation.h"

namespace flame
{
	struct Bitmap
	{
		virtual void release() = 0;

		virtual uint get_width() const = 0;
		virtual uint get_height() const = 0;
		virtual uint get_channel() const = 0;
		virtual uint get_byte_per_channel() const = 0;
		virtual uint get_pitch() const = 0;
		virtual uchar* get_data() const = 0;
		virtual uint get_size() const = 0;
		virtual bool get_srgb() const = 0;

		virtual void swap_channel(uint ch1, uint ch2) = 0;
		virtual void copy_to(BitmapPtr dst, uint width, uint height, uint src_x = 0, uint src_y = 0, uint dst_x = 0, uint dst_y = 0, bool border = false) = 0;
		virtual void srgb_to_linear() = 0;
		virtual void save(const wchar_t* filename) = 0;

		FLAME_FOUNDATION_EXPORTS static Bitmap* create(uint width, uint height, uint channel = 4, uint byte_per_channel = 1, uchar* data = nullptr);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create(const wchar_t* filename);
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
