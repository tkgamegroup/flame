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

		struct GlyphPrivate : Glyph
		{
			ushort code = 0;
			Vec2i off = Vec2i(0);
			Vec2u size = Vec2u(0);
			Vec4f uv = Vec4f(0.f);
			int advance = 0;

			ushort get_code() const override { return code; }
			Vec2i get_off() const override { return off; }
			Vec2u get_size() const override { return size; }
			Vec4f get_uv() const override { return uv; }
			int get_advance() const override { return advance; }
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

		struct FontAtlasBridge : FontAtlas
		{
			Glyph* get_glyph(wchar_t code, uint size) override;
		};

		struct FontAtlasPrivate : FontAtlas
		{
			std::vector<FontPrivate*> fonts;

			std::unordered_map<GlyphKey, std::unique_ptr<GlyphPrivate>, Hasher_GlyphKey> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			DevicePrivate* device;
			std::unique_ptr<ImagePrivate> image;
			ImageViewPrivate* view;

			FontAtlasPrivate(DevicePrivate* device, std::span<FontPrivate*> fonts);

			void release() override { delete this; }

			GlyphPrivate* get_glyph(wchar_t code, uint size);

			ImageView* get_view() const override { return view; }
		};

		inline Glyph* FontAtlasBridge::get_glyph(wchar_t code, uint size)
		{
			return ((FontAtlasPrivate*)this)->get_glyph(code, size);
		}
	}
}
