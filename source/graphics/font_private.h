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
			std::string file;
			stbtt_fontinfo* stbtt_info;

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
			std::vector<std::unique_ptr<Font>> fonts;

			std::unordered_map<GlyphKey, Glyph, Hasher_GlyphKey> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			DevicePtr device;
			std::unique_ptr<ImagePrivate> image;
			ImageViewPtr view;

			Glyph empty_glyph;

			FontAtlasPrivate(DevicePtr device, const std::vector<Font*>& fonts);

			void release() override { delete this; }

			const Glyph& get_glyph(wchar_t code, uint size) override;

			ImageViewPtr get_view() const override { return view; }

			static FontAtlasPtr get(DevicePtr device, const std::wstring& res);
		};
	}
}
