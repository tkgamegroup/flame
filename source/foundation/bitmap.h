#pragma once

#include "foundation.h"

namespace flame
{
	struct Bitmap
	{
		uvec2 extent;
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
		virtual void copy_to(BitmapPtr dst, const uvec2& extent, const ivec2& src_off, const ivec2& dst_off, bool border = false) = 0;
		virtual void srgb_to_linear() = 0;
		virtual void save(const std::filesystem::path& filename) = 0;

		struct Create
		{
			virtual BitmapPtr operator()(const uvec2& extent, uint chs = 4, uint bits_per_ch = 8, uchar* data = nullptr) = 0;
			virtual BitmapPtr operator()(const std::filesystem::path& filename, int req_ch = 0) = 0;
		};
		FLAME_FOUNDATION_API static Create& create;
	};

	struct BinPackNode
	{
		bool used = false;
		ivec2 pos = ivec2(0);
		ivec2 extent;
		std::unique_ptr<BinPackNode> right;
		std::unique_ptr<BinPackNode> bottom;

		BinPackNode(const ivec2& extent) :
			extent(extent)
		{
		}

		BinPackNode* find(const ivec2& sz)
		{
			if (!used && extent.x >= sz.x && extent.y >= sz.y)
			{
				used = true;
				if (extent.x > sz.x)
				{
					right.reset(new BinPackNode(ivec2(extent.x - sz.x, sz.y)));
					right->pos = ivec2(pos.x + sz.x, pos.y);
				}
				if (extent.y > sz.y)
				{
					bottom.reset(new BinPackNode(ivec2(extent.x, extent.y - sz.y)));
					bottom->pos = ivec2(pos.x, pos.y + sz.y);
				}
				return this;
			}
			if (auto n = right ? right->find(sz) : nullptr; n)
				return n;
			if (auto n = bottom ? bottom->find(sz) : nullptr; n)
				return n;
			return nullptr;
		}
	};
}
