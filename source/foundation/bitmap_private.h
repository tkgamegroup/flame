#pragma once

#include <flame/foundation/bitmap.h>

namespace flame
{
	struct BitmapPrivate : Bitmap
	{
		uint width;
		uint height;
		uint channel;
		uint byte_per_channel;
		uint pitch;
		uchar* data;
		uint size;
		bool srgb;

		~BitmapPrivate();
		void release() override { delete this; }
		uint get_width() const override { return width; }
		uint get_height() const override { return height; }
		uint get_channel() const override { return channel; }
		uint get_byte_per_channel() const override { return byte_per_channel; }
		uint get_pitch() const override { return pitch; }
		uchar* get_data() const override { return data; }
		uint get_size() const override { return size; }
		bool get_srgb() const override { return srgb; }
		void add_alpha_channel() override;
		void swap_channel(uint ch1, uint ch2);
		void copy_to(Bitmap* _dst, uint w, uint h, uint src_x, uint src_y, uint _dst_x, uint _dst_y, bool border) override;
		void save_to_file(const wchar_t* filename) override;
	};
}
