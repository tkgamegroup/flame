#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Bitmap
	{
		Vec2u size;
		uint channel;
		uint bpp;
		bool srgb;
		uint pitch;
		uchar* data;
		uint data_size;

		static uint get_pitch(uint line_bytes)
		{
			auto p = line_bytes;
			if (p % 4 == 0)
				return p;
			return p + 4 - p % 4;
		}

		static uint get_pitch(uint cx, uint bpp)
		{
			return get_pitch(cx * (bpp / 8));
		}

		void calc_pitch()
		{
			pitch = get_pitch(size.x(), bpp);
		}

		FLAME_FOUNDATION_EXPORTS void add_alpha_channel();
		FLAME_FOUNDATION_EXPORTS void swap_channel(uint ch1, uint ch2);
		FLAME_FOUNDATION_EXPORTS void copy_to(Bitmap* b, const Vec2u& src_off, const Vec2u& cpy_size, const Vec2u& dst_off, bool border = false);

		FLAME_FOUNDATION_EXPORTS static Bitmap* create(const Vec2u& size, uint channel, uint bpp, uchar* data = nullptr);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_with_plaincolor(const Vec2u& size, const Vec4c& color);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_with_horizontalstripes_pattern(const Vec2u& size, uint offset, uint line_width, uint spacing, const Vec4c& foreground_color, const Vec4c& background_color);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_with_verticalstripes_pattern(const Vec2u& size, uint offset, uint line_width, uint spacing, const Vec4c& foreground_color, const Vec4c& background_color);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_with_brickwall_pattern(const Vec2u& size, const Vec2u& offset, uint line_width, uint half_brick_width, uint brick_height, const Vec4c& foreground_color, const Vec4c& background_color);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_from_file(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static Bitmap* create_from_gif(const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void save_to_file(Bitmap* b, const std::wstring& filename);
		FLAME_FOUNDATION_EXPORTS static void destroy(Bitmap* b);
	};

	FLAME_FOUNDATION_EXPORTS Mail<std::pair<Bitmap*, std::vector<std::pair<std::string, Vec4u>>>> bin_pack(const std::vector<std::pair<Bitmap*, std::string>>& textures);
}
