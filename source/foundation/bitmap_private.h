#pragma once

#include <flame/foundation/bitmap.h>

namespace flame
{
	struct BitmapPrivate : Bitmap
	{
		uint _width;
		uint _height;
		uint _channel;
		uint _byte_per_channel;
		uint _pitch;
		uchar* _data;
		uint _size;
		bool _srgb;

		BitmapPrivate(uint width, uint height, uint channel = 4, uint byte_per_channel = 1, uchar* data = nullptr);
		~BitmapPrivate();

		void release() override { delete this; }

		void _add_alpha_channel();
		void _swap_channel(uint ch1, uint ch2);
		void _copy_to(BitmapPrivate* dst, uint w, uint h, uint src_x, uint src_y, uint dst_x, uint dst_y, bool border);
		void _save(const std::filesystem::path& filename);

		static BitmapPrivate* _create(const std::filesystem::path& filename);

		uint get_width() const override { return _width; }
		uint get_height() const override { return _height; }
		uint get_channel() const override { return _channel; }
		uint get_byte_per_channel() const override { return _byte_per_channel; }
		uint get_pitch() const override { return _pitch; }
		uchar* get_data() const override { return _data; }
		uint get_size() const override { return _size; }
		bool get_srgb() const override { return _srgb; }

		void add_alpha_channel() override { _add_alpha_channel(); }
		void swap_channel(uint ch1, uint ch2) override { _swap_channel(ch1, ch2); }
		void copy_to(Bitmap* dst, uint w, uint h, uint src_x, uint src_y, uint dst_x, uint dst_y, bool border) override { _copy_to((BitmapPrivate*)dst, w, h, src_x, src_y, dst_x, dst_y, border); }
		void save(const wchar_t* filename) override { _save(filename); }
	};
}
