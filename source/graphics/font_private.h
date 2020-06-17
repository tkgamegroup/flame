#pragma once

#include <flame/graphics/font.h>

struct stbtt_fontinfo;

namespace flame
{
	struct BinPackNode;

	namespace graphics
	{
		struct ImagePrivate;

		struct FontPrivate : Font
		{
			std::filesystem::path filename;
			std::string file;
			std::unique_ptr<stbtt_fontinfo> stbtt_info;

			FontPrivate(const std::wstring& filename);

			void release() override { delete this; }

			const wchar_t* get_filename() const override { return filename.c_str(); }
		};

		struct GlyphPrivate : Glyph
		{
			ushort code;
			Vec2i off;
			Vec2u size;
			Vec4f uv;
			int advance;

			ushort get_code() const override { return code; }
			Vec2i get_off() const override { return off; }
			Vec2u get_size() const override { return size; }
			Vec4f get_uv() const override { return uv; }
			int get_advance() const override { return advance; }
		};

		struct FontAtlasPrivate : FontAtlas
		{
			int slot;

			std::vector<FontPrivate*> fonts;

			std::unique_ptr<Glyph> empty_glyph;
			std::unordered_map<uint, std::unique_ptr<Glyph>> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			std::unique_ptr<ImagePrivate> image;
			std::unique_ptr<ImageviewPrivate> view;

			FontAtlasPrivate(DevicePrivate* d, std::span<FontPrivate*> fonts);
			~FontAtlasPrivate();

			void release() override { delete this; }

			Glyph* get_glyph(wchar_t unicode, uint font_size) override;
		};
	}
}
