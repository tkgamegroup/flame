#pragma once

#include <flame/graphics/font.h>

namespace flame
{
	namespace graphics
	{
		struct ImagePrivate;

		struct Font
		{
			std::filesystem::path filename;
			std::string font_file;
			stbtt_fontinfo info;
			uint ref_count;

			Font(const std::wstring& filename);
		};

		struct FontAtlasPrivate : FontAtlas
		{
			int slot;

			std::vector<Font*> fonts;

			std::unique_ptr<Glyph> empty_glyph;
			std::unordered_map<uint, std::unique_ptr<Glyph>> map;
			std::unique_ptr<BinPackNode> bin_pack_root;

			std::unique_ptr<ImagePrivate> image;
			std::unique_ptr<ImageviewPrivate> view;

			FontAtlasPrivate(DevicePrivate* d, uint font_count, const wchar_t* const* _fonts);
			~FontAtlasPrivate();

			void release() override;

			const Glyph* get_glyph(wchar_t unicode, uint font_size) override;
		};
	}
}
