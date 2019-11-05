#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	struct Bitmap;

	namespace graphics
	{
		struct Device;

		struct Image
		{
			Format$ format;
			Vec2u size;
			uint level;
			uint layer;
			SampleCount$ sample_count;

			uint channel;
			uint bpp;
			uint pitch;
			uint data_size;

			FLAME_GRAPHICS_EXPORTS static Format$ find_format(uint channel, uint bpp);

			// a layout change from Undefined to ShaderReadOnly, and a clear
			FLAME_GRAPHICS_EXPORTS void init(const Vec4c& col);

			// accepted formats for get/set pixels: Format_R8G8B8A8_UNORM, Format_R16G16B16A16_UNORM
			FLAME_GRAPHICS_EXPORTS void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst);
			FLAME_GRAPHICS_EXPORTS void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src);

			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, SampleCount$ sample_count, ImageUsage$ usage, void* data = nullptr);
			FLAME_GRAPHICS_EXPORTS static Image* create_from_bitmap(Device* d, Bitmap* bmp, ImageUsage$ extra_usage = ImageUsage$(0)); // default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static Image* create_from_file(Device* d, const std::wstring& filename, ImageUsage$ extra_usage = ImageUsage$(0)); // default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static void save_to_png(Image *i, const std::wstring& filename);
			FLAME_GRAPHICS_EXPORTS static Image* create_from_native(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, void* native);

			FLAME_GRAPHICS_EXPORTS static void destroy(Image* i);
		};

		struct Imageview
		{
			ImageviewType$ type;
			uint base_level;
			uint level_count;
			uint base_layer;
			uint layer_count;
			Swizzle$ swizzle_r;
			Swizzle$ swizzle_g;
			Swizzle$ swizzle_b;
			Swizzle$ swizzle_a;

			FLAME_GRAPHICS_EXPORTS Image* image() const;

			FLAME_GRAPHICS_EXPORTS static Imageview* create(Image* i, ImageviewType$ type = Imageview2D, uint base_level = 0, uint level_count = 1, uint base_layer = 0, uint layer_count = 1, 
				Swizzle$ swizzle_r = SwizzleIdentity, Swizzle$ swizzle_g = SwizzleIdentity, Swizzle$ swizzle_b = SwizzleIdentity, Swizzle$ swizzle_a = SwizzleIdentity);
			FLAME_GRAPHICS_EXPORTS static void destroy(Imageview* v);
		};

		inline Image* image_from_target(TargetType$ t, void* v)
		{
			if (!v)
				return nullptr;

			switch (t)
			{
			case TargetImage:
				return (Image*)v;
			case TargetImageview:
				return ((Imageview*)v)->image();
			case TargetImages:
			{
				auto& vec = *(std::vector<Image*>*)v;
				return vec.empty() ? nullptr : vec[0];
			}
			}
			return nullptr;
		}

		struct Sampler
		{
			FLAME_GRAPHICS_EXPORTS static Sampler* create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
			FLAME_GRAPHICS_EXPORTS static void destroy(Sampler* s);
		};

		struct Atlas
		{
			bool border;

			struct Region
			{
				std::wstring filename;
				Vec2i pos;
				Vec2i size;
				Vec2f uv0;
				Vec2f uv1;
			};

			FLAME_GRAPHICS_EXPORTS Imageview* imageview() const;
			FLAME_GRAPHICS_EXPORTS const std::vector<Region>& regions() const;

			int find_region(const std::wstring& filename) const
			{
				for (auto i = 0; i < regions().size(); i++)
				{
					if (regions()[i].filename == filename)
						return i;
				}
				return -1;
			}

			FLAME_GRAPHICS_EXPORTS static Atlas* load(Device* d, const std::wstring& filename /* the image filename, not the atlas */);
			FLAME_GRAPHICS_EXPORTS static void destroy(Atlas* a);
		};
	}
}

