#include "bitmap_private.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace flame
{
	BitmapPrivate::~BitmapPrivate()
	{
		delete[]data;
	}

	void BitmapPrivate::change_format(uint _chs)
	{
		data = stbi__convert_format(data, chs, _chs, size.x, size.y);
		chs = _chs;
		bpp = _chs * 8;
		pitch = image_pitch(size.x * (bpp / 8));
		data_size = pitch * size.y;
	}

	void BitmapPrivate::swap_channel(uint ch1, uint ch2)
	{
		assert(bpp == 32 && ch1 < chs && ch2 < chs);
		for (auto j = 0; j < size.y; j++)
		{
			auto line = data + j * pitch;
			for (auto i = 0; i < size.x; i++)
			{
				auto p = line + i * chs;
				std::swap(p[ch1], p[ch2]);
			}
		}
	}

	void BitmapPrivate::copy_to(BitmapPtr dst, const uvec2& s, const ivec2& src_off, const ivec2& _dst_off, bool border)
	{
		auto b1 = border ? 1 : 0;
		assert(bpp == 32 && chs == dst->chs && bpp == dst->bpp &&
			all(lessThanEqual(src_off + ivec2(s), ivec2(size))) && 
			all(lessThanEqual(_dst_off + ivec2(s) + b1 * 2, ivec2(dst->size))));

		auto dst_off = _dst_off + b1;
		auto dst_data = dst->data;
		auto dst_pitch = dst->pitch;
		for (auto i = 0; i < s.y; i++)
		{
			auto src_line = data + (src_off.y + i) * pitch + src_off.x * chs;
			auto dst_line = dst_data + (dst_off.y + i) * dst_pitch + dst_off.x * chs;
			memcpy(dst_line, src_line, s.x * chs);
		}

		if (border)
		{
			memcpy(dst->data + (dst_off.y - 1) * dst_pitch + dst_off.x * chs, data + src_off.y * pitch + src_off.x * chs, s.x * chs); // top line
			memcpy(dst->data + (dst_off.y + s.y) * dst_pitch + dst_off.x * chs, data + (src_off.y + s.y - 1) * pitch + src_off.x * chs, s.x * chs); // bottom line
			for (auto i = 0; i < s.y; i++)
				memcpy(dst->data + (dst_off.y + i) * dst_pitch + (dst_off.x - 1) * chs, data + (src_off.y + i) * pitch + src_off.x * chs, chs); // left line
			for (auto i = 0; i < s.y; i++)
				memcpy(dst->data + (dst_off.y + i) * dst_pitch + (dst_off.x + s.x) * chs, data + (src_off.y + i) * pitch + (src_off.x + s.x - 1) * chs, chs); // left line

			memcpy(dst->data + (dst_off.y - 1) * dst_pitch + (dst_off.x - 1) * chs, data + src_off.y * pitch + src_off.x * chs, chs); // left top corner
			memcpy(dst->data + (dst_off.y - 1) * dst_pitch + (dst_off.x + s.x) * chs, data + src_off.y * pitch + (src_off.x + s.x - 1) * chs, chs); // right top corner
			memcpy(dst->data + (dst_off.y + s.y) * dst_pitch + (dst_off.x - 1) * chs, data + (src_off.y + s.y - 1) * pitch + src_off.x * chs, chs); // left bottom corner
			memcpy(dst->data + (dst_off.y + s.y) * dst_pitch + (dst_off.x + s.x) * chs, data + (src_off.y + s.y - 1) * pitch + (src_off.x + s.x - 1) * chs, chs); // right bottom corner
		}
	}

	void BitmapPrivate::srgb_to_linear()
	{
		assert(chs >= 3);
		for (auto j = 0; j < size.y; j++)
		{
			auto line = data + j * pitch;
			for (auto i = 0; i < size.x; i++)
			{
				auto p = line + i * chs;
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
			stbi_write_png(filename.string().c_str(), size.x, size.y, chs, data, pitch);
		else if (ext == L".bmp")
			stbi_write_bmp(filename.string().c_str(), size.x, size.y, chs, data);
		else if (ext == L".jpg" || ext == L".jpeg")
			stbi_write_jpg(filename.string().c_str(), size.x, size.y, chs, data, 0);
	}

	struct BitmapCreate : Bitmap::Create
	{
		BitmapPtr operator()(const uvec2& size, uint chs, uint bits_per_ch, uchar* data) override
		{
			auto ret = new BitmapPrivate;
			ret->size = size;
			ret->chs = chs;
			ret->bpp = bits_per_ch * chs;
			ret->pitch = image_pitch(size.x * (ret->bpp / 8));
			ret->data_size = ret->pitch * size.y;
			ret->data = new uchar[ret->data_size];
			if (!data)
				memset(ret->data, 0, ret->data_size);
			else
				memcpy(ret->data, data, ret->data_size);
			ret->srgb = false;
			return ret;
		}

		BitmapPtr operator()(const std::filesystem::path& filename, int req_ch) override
		{
			if (!std::filesystem::exists(filename))
				return nullptr;

			auto file = _wfopen(filename.c_str(), L"rb");
			if (!file)
				return nullptr;

			int cx, cy, chs;
			auto data = stbi_load_from_file(file, &cx, &cy, &chs, req_ch);
			if (!data)
				return nullptr;
			fclose(file);
			if (req_ch) chs = req_ch;
			auto ret = Bitmap::create(uvec2(cx, cy), chs, 8, data);
			stbi_image_free(data);
			return ret;
		}
	}Bitmap_create;
	Bitmap::Create& Bitmap::create = Bitmap_create;
}
