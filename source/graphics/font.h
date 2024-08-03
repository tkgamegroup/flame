#pragma once

#include"graphics.h"

namespace flame
{
	namespace graphics
	{
		enum FontAtlasType
		{
			FontAtlasBitmap,
			FontAtlasSDF
		};

		enum
		{
			SDF_FONT_SIZE = 32U
		};

		// Reflect
		struct FontAtlas
		{
			FontAtlasType type;
			ImageViewPtr view;

			std::vector<std::filesystem::path> font_names;
			uint ref = 0;

			virtual ~FontAtlas() {}

			virtual const Glyph& get_glyph(wchar_t unicode, uint font_size) = 0;

			inline float get_scale(uint font_size)
			{
				if (type == FontAtlasBitmap)
					return 1.f;
				if (type == FontAtlasSDF)
					return (float)font_size / (float)SDF_FONT_SIZE;
				return 1.f;
			}

			struct Get
			{
				virtual FontAtlasPtr operator()(const std::vector<std::filesystem::path>& font_names, FontAtlasType type = FontAtlasBitmap) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(FontAtlasPtr atlas) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};
	}
}
