#include <flame/serialize.h>
#include "bitmap_private.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace flame
{
	BitmapPrivate::BitmapPrivate(uint width, uint height, uint channel, uint byte_per_channel, uchar* data) :
		_width(width),
		_height(height),
		_channel(channel),
		_byte_per_channel(byte_per_channel)
	{
		_pitch = image_pitch(width * channel * byte_per_channel);
		_size = _pitch * height;
		_data = new uchar[_size];

		if (!data)
			memset(_data, 0, _size);
		else
			memcpy(_data, data, _size);
		_srgb = false;
	}

	BitmapPrivate::~BitmapPrivate()
	{
		delete[] _data;
	}

	void BitmapPrivate::_add_alpha_channel()
	{
		assert(_channel == 3 && _byte_per_channel == 1);
		if (_channel != 3 || _byte_per_channel != 1)
			return;

		auto new_data = new uchar[_width * _height * 4];
		auto dst = new_data;
		for (auto j = 0; j < _height; j++)
		{
			auto src = _data + j * _pitch;
			for (auto i = 0; i < _width; i++)
			{
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = 255;
			}
		}
		_channel = 4;
		_pitch = image_pitch(_width * _channel * _byte_per_channel);
		_size = _pitch * _height;
		delete[]_data;
		_data = new_data;
	}

	void BitmapPrivate::_swap_channel(uint ch1, uint ch2)
	{
		assert(_byte_per_channel == 1 && ch1 < _channel&& ch2 < _channel);
		if (_byte_per_channel != 1 || ch1 >= _channel || ch2 >= _channel)
			return;

		for (auto j = 0; j < _height; j++)
		{
			auto line = _data + j * _pitch;
			for (auto i = 0; i < _width; i++)
			{
				auto p = line + i * _channel;
				std::swap(p[ch1], p[ch2]);
			}
		}
	}

	void BitmapPrivate::_copy_to(BitmapPrivate* dst, uint w, uint h, uint src_x, uint src_y, uint _dst_x, uint _dst_y, bool border)
	{
		auto b1 = border ? 1 : 0;
		auto b2 = b1 << 1;
		assert(_byte_per_channel == 1 &&
			_channel == dst->_channel &&
			_byte_per_channel == dst->_byte_per_channel &&
			src_x + w <= _width && src_y + h <= _height &&
			_dst_x + w + b2 <= dst->_width && _dst_y + h + b2 <= dst->_height);
		if (_byte_per_channel != 1 ||
			_channel != dst->_channel ||
			_byte_per_channel != dst->_byte_per_channel ||
			src_x + w > _width || src_y + h > _height ||
			_dst_x + w + b2 > dst->_width || _dst_y + h + b2 > dst->_height)
			return;

		auto dst_x = _dst_x + b1;
		auto dst_y = _dst_y + b2;
		auto dst_data = dst->_data;
		auto dst_pitch = dst->_pitch;
		for (auto i = 0; i < h; i++)
		{
			auto src_line = _data + (src_y + i) * _pitch + src_x * _channel;
			auto dst_line = dst_data + (dst_y + i) * dst_pitch + dst_x * _channel;
			memcpy(dst_line, src_line, w * _channel);
		}

		if (border)
		{
			memcpy(dst->_data + (dst_y - 1) * dst_pitch + dst_x * _channel, _data + src_y * _pitch + src_x * _channel, w * _channel); // top line
			memcpy(dst->_data + (dst_y + h) * dst_pitch + dst_x * _channel, _data + (src_y + h - 1) * _pitch + src_x * _channel, w * _channel); // bottom line
			for (auto i = 0; i < h; i++)
				memcpy(dst->_data + (dst_y + i) * dst_pitch + (dst_x - 1) * _channel, _data + (src_y + i) * _pitch + src_x * _channel, _channel); // left line
			for (auto i = 0; i < h; i++)
				memcpy(dst->_data + (dst_y + i) * dst_pitch + (dst_x + w) * _channel, _data + (src_y + i) * _pitch + (src_x + w - 1) * _channel, _channel); // left line

			memcpy(dst->_data + (dst_y - 1) * dst_pitch + (dst_x - 1) * _channel, _data + src_y * _pitch + src_x * _channel, _channel); // left top corner
			memcpy(dst->_data + (dst_y - 1) * dst_pitch + (dst_x + w) * _channel, _data + src_y * _pitch + (src_x + w - 1) * _channel, _channel); // right top corner
			memcpy(dst->_data + (dst_y + h) * dst_pitch + (dst_x - 1) * _channel, _data + (src_y + h - 1) * _pitch + src_x * _channel, _channel); // left bottom corner
			memcpy(dst->_data + (dst_y + h) * dst_pitch + (dst_x + w) * _channel, _data + (src_y + h - 1) * _pitch + (src_x + w - 1) * _channel, _channel); // right bottom corner
		}
	}

	void BitmapPrivate::_save(const std::filesystem::path& filename)
	{
		auto ext = std::filesystem::path(filename).extension();

		if (ext == L".png")
			stbi_write_png(w2s(filename).c_str(), _width, _height, _channel, _data, _pitch);
		else if (ext == L".bmp")
			stbi_write_bmp(w2s(filename).c_str(), _width, _height, _channel, _data);
	}

	BitmapPrivate* BitmapPrivate::_create(const std::filesystem::path& filename)
	{
		if (!std::filesystem::exists(filename))
			return nullptr;

		int cx, cy, _channel;
		auto data = stbi_load(filename.string().c_str(), &cx, &cy, &_channel, 4);
		if (!data)
			return nullptr;
		auto b = new BitmapPrivate(cx, cy, 4, 1, data);
		stbi_image_free(data);
		return b;
	}
}
