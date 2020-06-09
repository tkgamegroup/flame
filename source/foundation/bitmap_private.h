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
		void release() override;
		uint get_width() const override;
		uint get_height() const override;
		uint get_channel() const override;
		uint get_byte_per_channel() const override;
		uint get_pitch() const override;
		uchar* get_data() const override;
		uint get_size() const override;
		bool get_srgb() const override;
		void add_alpha_channel() override;
		void swap_channel(uint ch1, uint ch2);
		void copy_to(Bitmap* _dst, uint w, uint h, uint src_x, uint src_y, uint _dst_x, uint _dst_y, bool border) override;
		void save_to_file(const wchar_t* filename) override;
	};
}
