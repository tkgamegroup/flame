#pragma once

#include <flame/foundation/bitmap.h>
#include <flame/graphics/font.h>

struct stbtt_fontinfo;

namespace flame
{
	namespace graphics
	{
		struct ImagePrivate;

		struct FontPrivate : Font
		{
			std::filesystem::path _filename;
			std::string _file;
			stbtt_fontinfo* _stbtt_info;

			FontPrivate(const std::wstring& filename);
			~FontPrivate();

			void release() override { delete this; }

			const wchar_t* get_filename() const override { return _filename.c_str(); }
		};

		struct GlyphPrivate : Glyph
		{
			ushort _code = 0;
			Vec2i _off = Vec2i(0);
			Vec2u _size = Vec2u(0);
			Vec4f _uv = Vec4f(0.f);
			int _advance = 0;

			ushort get_code() const override { return _code; }
			Vec2i get_off() const override { return _off; }
			Vec2u get_size() const override { return _size; }
			Vec4f get_uv() const override { return _uv; }
			int get_advance() const override { return _advance; }
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
			std::vector<FontPrivate*> _fonts;

			std::unordered_map<GlyphKey, std::unique_ptr<GlyphPrivate>, Hasher_GlyphKey> _map;
			std::unique_ptr<BinPackNode> _bin_pack_root;

			std::unique_ptr<ImagePrivate> _image;
			std::unique_ptr<ImageviewPrivate> _view;

			FontAtlasPrivate(DevicePrivate* d, std::span<FontPrivate*> fonts);

			void release() override { delete this; }

			GlyphPrivate* _get_glyph(wchar_t code, uint size);

			Glyph* get_glyph(wchar_t code, uint size) override { return _get_glyph(code, size); }
		};
	}
}
