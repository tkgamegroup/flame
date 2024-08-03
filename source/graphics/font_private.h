#pragma once

#include "../foundation/bitmap.h"
#include "font.h"

struct stbtt_fontinfo;

namespace flame
{
	namespace graphics
	{
		struct Font
		{
			std::filesystem::path filename;
			std::string content;
			stbtt_fontinfo* stbtt_info;

			uint ref = 0;

			~Font();
		};

		struct FontAtlasPrivate : FontAtlas
		{
			std::vector<Font*> fonts;

			std::vector<std::vector<Glyph>> glyphs;
			std::unique_ptr<BinPackNode> bin_pack_root;

			std::unique_ptr<ImagePrivate> image;

			~FontAtlasPrivate();

			Glyph pack_glyph(wchar_t code, uint font_size);
			const Glyph& get_glyph(wchar_t code, uint font_size) override;
		};
	}
}
