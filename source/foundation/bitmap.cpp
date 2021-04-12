#include "bitmap_private.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace flame
{
	BitmapPrivate::BitmapPrivate(uint width, uint height, uint channel, uint byte_per_channel, uchar* _data) :
		width(width),
		height(height),
		channel(channel),
		byte_per_channel(byte_per_channel)
	{
		pitch = image_pitch(width * channel * byte_per_channel);
		size = pitch * height;
		data = new uchar[size];

		if (!_data)
			memset(data, 0, size);
		else
			memcpy(data, _data, size);
		srgb = false;
	}

	BitmapPrivate::~BitmapPrivate()
	{
		delete[]data;
	}

	void BitmapPrivate::swap_channel(uint ch1, uint ch2)
	{
		fassert(byte_per_channel == 1 && ch1 < channel&& ch2 < channel);
		if (byte_per_channel != 1 || ch1 >= channel || ch2 >= channel)
			return;

		for (auto j = 0; j < height; j++)
		{
			auto line = data + j * pitch;
			for (auto i = 0; i < width; i++)
			{
				auto p = line + i * channel;
				std::swap(p[ch1], p[ch2]);
			}
		}
	}

	void BitmapPrivate::copy_to(BitmapPtr dst, uint w, uint h, uint src_x, uint src_y, uint _dst_x, uint _dst_y, bool border)
	{
		auto b1 = border ? 1 : 0;
		auto b2 = b1 << 1;
		fassert(byte_per_channel == 1 &&
			channel == dst->channel &&
			byte_per_channel == dst->byte_per_channel &&
			src_x + w <= width && src_y + h <= height &&
			_dst_x + w + b2 <= dst->width && _dst_y + h + b2 <= dst->height);
		if (byte_per_channel != 1 ||
			channel != dst->channel ||
			byte_per_channel != dst->byte_per_channel ||
			src_x + w > width || src_y + h > height ||
			_dst_x + w + b2 > dst->width || _dst_y + h + b2 > dst->height)
			return;

		auto dst_x = _dst_x + b1;
		auto dst_y = _dst_y + b2;
		auto dst_data = dst->data;
		auto dst_pitch = dst->pitch;
		for (auto i = 0; i < h; i++)
		{
			auto src_line = data + (src_y + i) * pitch + src_x * channel;
			auto dst_line = dst_data + (dst_y + i) * dst_pitch + dst_x * channel;
			memcpy(dst_line, src_line, w * channel);
		}

		if (border)
		{
			memcpy(dst->data + (dst_y - 1) * dst_pitch + dst_x * channel, data + src_y * pitch + src_x * channel, w * channel); // top line
			memcpy(dst->data + (dst_y + h) * dst_pitch + dst_x * channel, data + (src_y + h - 1) * pitch + src_x * channel, w * channel); // bottom line
			for (auto i = 0; i < h; i++)
				memcpy(dst->data + (dst_y + i) * dst_pitch + (dst_x - 1) * channel, data + (src_y + i) * pitch + src_x * channel, channel); // left line
			for (auto i = 0; i < h; i++)
				memcpy(dst->data + (dst_y + i) * dst_pitch + (dst_x + w) * channel, data + (src_y + i) * pitch + (src_x + w - 1) * channel, channel); // left line

			memcpy(dst->data + (dst_y - 1) * dst_pitch + (dst_x - 1) * channel, data + src_y * pitch + src_x * channel, channel); // left top corner
			memcpy(dst->data + (dst_y - 1) * dst_pitch + (dst_x + w) * channel, data + src_y * pitch + (src_x + w - 1) * channel, channel); // right top corner
			memcpy(dst->data + (dst_y + h) * dst_pitch + (dst_x - 1) * channel, data + (src_y + h - 1) * pitch + src_x * channel, channel); // left bottom corner
			memcpy(dst->data + (dst_y + h) * dst_pitch + (dst_x + w) * channel, data + (src_y + h - 1) * pitch + (src_x + w - 1) * channel, channel); // right bottom corner
		}
	}

	void BitmapPrivate::srgb_to_linear()
	{
		fassert(channel >= 3);
		if (channel < 3)
			return;
		for (auto j = 0; j < height; j++)
		{
			auto line = data + j * pitch;
			for (auto i = 0; i < width; i++)
			{
				auto p = line + i * channel;
				{
					auto& r = p[0];
					r = pow(r / 255.f, 2.2f) * 255.f;
				}
				{
					auto& g = p[1];
					g = pow(g / 255.f, 2.2f) * 255.f;
				}
				{
					auto& b = p[2];
					b = pow(b / 255.f, 2.2f) * 255.f;
				}
			}
		}
	}

	void BitmapPrivate::save(const std::filesystem::path& filename)
	{
		auto ext = std::filesystem::path(filename).extension();

		if (ext == L".png")
			stbi_write_png(w2s(filename).c_str(), width, height, channel, data, pitch);
		else if (ext == L".bmp")
			stbi_write_bmp(w2s(filename).c_str(), width, height, channel, data);
	}

	BitmapPrivate* BitmapPrivate::create(const std::filesystem::path& filename)
	{
		if (!std::filesystem::exists(filename))
			return nullptr;

		int cx, cy, channel;
		
		auto data = stbi_load(filename.string().c_str(), &cx, &cy, &channel, 0);
		if (!data)
			return nullptr;
		if (channel == 3)
		{
			data = stbi__convert_format(data, 3, 4, cx, cy);
			channel = 4;
		}
		auto b = new BitmapPrivate(cx, cy, channel, 1, data);
		stbi_image_free(data);
		return b;
	}

	Bitmap* Bitmap::create(uint width, uint height, uint channel, uint byte_per_channel, uchar* data) { return new BitmapPrivate(width, height, channel, byte_per_channel, data); }
	Bitmap* Bitmap::create(const wchar_t* filename) { return BitmapPrivate::create(filename); }
}
