#pragma once

#include <flame/foundation/bitmap.h>
#include <flame/graphics/font.h>

#include <stb_truetype.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct ImagePrivate;

		struct Font
		{
			std::string file;
			stbtt_fontinfo stbtt_info;
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

			DevicePrivate* device;
			std::unique_ptr<ImagePrivate> image;
			ImageViewPrivate* view;

			Glyph empty_glyph;

			FontAtlasPrivate(DevicePrivate* device, const std::vector<Font*>& fonts);

			void release() override { delete this; }

			const Glyph& get_glyph(wchar_t code, uint size) override;

			ImageView* get_view() const override { return view; }

			static FontAtlasPrivate* get(DevicePrivate* device, const std::wstring& res);
		};
	}
}
