#pragma once

#include <flame/foundation/bitmap.h>
#include <flame/graphics/font.h>

struct stbtt_fontinfo;

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct ImagePrivate;

		struct FontPrivate : Font
		{
			std::filesystem::path filename;
			std::string file;
			stbtt_fontinfo* stbtt_info;

			FontPrivate(const std::wstring& filename);
			~FontPrivate();

			void release() override { delete this; }

			const wchar_t* get_filename() const override { return filename.c_str(); }
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
			std::vector<FontPrivate*> fonts;

			std::unordered_map<GlyphKey, Glyph, Hasher_GlyphKey> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			DevicePrivate* device;
			std::unique_ptr<ImagePrivate> image;
			ImageViewPrivate* view;

			Glyph empty_glyph;

			FontAtlasPrivate(DevicePrivate* device, std::span<FontPrivate*> fonts);

			void release() override { delete this; }

			const Glyph& get_glyph(wchar_t code, uint size) override;

			ImageView* get_view() const override { return view; }
		};
	}
}
