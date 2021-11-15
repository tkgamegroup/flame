#pragma once

#include "bitmap.h"

namespace flame
{
	struct BitmapPrivate : Bitmap
	{
		BitmapPrivate(const uvec2& size, uint chs = 4, uint bpp = 32, uchar* data = nullptr);
		~BitmapPrivate();

		void change_format(uint chs) override;
		void swap_channel(uint ch1, uint ch2) override;
		void copy_to(BitmapPtr dst, const uvec2& size, const ivec2& src_off, const ivec2& dst_off, bool border) override;
		void srgb_to_linear() override;
		void save(const std::filesystem::path& filename) override;
	};
}
