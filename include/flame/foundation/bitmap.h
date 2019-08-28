#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Bitmap
	{
		Vec2u size;
		int channel;
		int bpp;
		bool sRGB;
		int pitch;
		unsigned char* data;
		int data_size;

		static int get_pitch(int line_bytes)
		{
			auto p = line_bytes;
			if (p % 4 == 0)
				return p;
			return p + 4 - p % 4;
		}

		static int get_pitch(int cx, int bpp)
		{
			return get_pitch(cx * (bpp / 8));
		}

		void calc_pitch()
		{
			pitch = get_pitch(size.x(), bpp);
		}

		FLAME_FOUNDATION_EXPORTS void add_alpha_channel();
		FLAME_FOUNDATION_EXPORTS void swap_channel(int ch1, int ch2);
		FLAME_FOUNDATION_EXPORTS void copy_to(Bitmap* b, const Vec2u& src_off, const Vec2u& cpy_size, const Vec2u& dst_off);

		FLAME_FOUNDATION_EXPORTS void save(const std::wstring& filename);

		FLAME_FOUNDATION_EXPORTS static Bitmap* create(const Vec2u& size, int channel, int bpp, unsigned char* data = nullptr, bool data_owner = false);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_from_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_from_gif(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(Bitmap* b);
	};

	FLAME_FOUNDATION_EXPORTS Bitmap* texture_bin_pack(const std::vector<Bitmap*>& textures);
}
