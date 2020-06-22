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

		struct FontAtlasPrivate : FontAtlas
		{
			int _slot = -1;

			std::vector<FontPrivate*> _fonts;

			std::unique_ptr<GlyphPrivate> _empty_glyph;
			std::unordered_map<uint, std::unique_ptr<GlyphPrivate>> _map;
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
