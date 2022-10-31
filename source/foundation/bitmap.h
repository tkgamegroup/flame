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
		uvec2 pos = uvec2(0);
		uvec2 extent;
		std::unique_ptr<BinPackNode> right;
		std::unique_ptr<BinPackNode> bottom;

		BinPackNode(const uvec2& extent) :
			extent(extent)
		{
		}

		BinPackNode* find(const uvec2& ext)
		{
			if (!used && extent.x >= ext.x && extent.y >= extent.y)
			{
				used = true;
				right.reset(new BinPackNode(uvec2(extent.x - ext.x, ext.y)));
				right->pos = pos + uvec2(ext.x, 0);
				bottom.reset(new BinPackNode(uvec2(extent.x, extent.y - ext.y)));
				bottom->pos = pos + uvec2(0, ext.y);
				return this;
			}
			if (!right || !bottom)
				return nullptr;
			auto n1 = right->find(ext);
			if (n1)
				return n1;
			auto n2 = bottom->find(ext);
			if (n2)
				return n2;
			return nullptr;
		}
	};
}
