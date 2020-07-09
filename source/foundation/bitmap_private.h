#pragma once

#include <flame/foundation/bitmap.h>

namespace flame
{
	struct BitmapPrivate__;

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

		BitmapPrivate__* operator->() { return (BitmapPrivate__*)this; }
		BitmapPrivate__* operator->() const { return (BitmapPrivate__*)this; }

		BitmapPrivate(uint width, uint height, uint channel = 4, uint byte_per_channel = 1, uchar* data = nullptr);
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
		void swap_channel(uint ch1, uint ch2) override;
		void copy_to(Bitmap* dst, uint w, uint h, uint src_x, uint src_y, uint dst_x, uint dst_y, bool border) override;
		void save(const wchar_t* filename) override;
	};

	struct BitmapPrivate__ : BitmapPrivate
	{
		void copy_to(BitmapPrivate* dst, uint w, uint h, uint src_x, uint src_y, uint dst_x, uint dst_y, bool border);
		void save(const std::filesystem::path& filename);

		static BitmapPrivate* create(const std::filesystem::path& filename);
	};
}
