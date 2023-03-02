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

		struct GlyphKey
		{
			ushort c;
			uint s;

			GlyphKey(ushort c, uint s) :
				c(c),
				s(s)
			{
			}

			bool operator==(const GlyphKey& rhs) const
			{
				return c == rhs.c && s == rhs.s;
			}
		};

		struct Hasher_GlyphKey
		{
			std::size_t operator()(const GlyphKey& k) const
			{
				return std::hash<short>()(k.c) ^ std::hash<int>()(k.s);
			}
		};

		struct FontAtlasPrivate : FontAtlas
		{
			std::vector<Font*> myfonts;

			std::unordered_map<GlyphKey, Glyph, Hasher_GlyphKey> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			std::unique_ptr<ImagePrivate> image;

			~FontAtlasPrivate();
			const Glyph& get_glyph(wchar_t code, uint font_size) override;
		};
	}
}
