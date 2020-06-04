#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	struct Bitmap;

	namespace graphics
	{
		struct Device;
		struct Imageview;

		inline Format get_image_format(uint channel, uint byte_per_channel)
		{
			switch (channel)
			{
			case 0:
				switch (byte_per_channel)
				{
				case 1:
					return Format_R8_UNORM;
				default:
					return Format_Undefined;

				}
				break;
			case 1:
				switch (byte_per_channel)
				{
				case 1:
					return Format_R8_UNORM;
				case 2:
					return Format_R16_UNORM;
				default:
					return Format_Undefined;
				}
				break;
			case 4:
				switch (byte_per_channel)
				{
				case 1:
					return Format_R8G8B8A8_UNORM;
				default:
					return Format_Undefined;
				}
				break;
			default:
				return Format_Undefined;
			}
		}

		struct Image
		{
			Format format;
			Vec2u size;
			uint level;
			uint layer;
			SampleCount sample_count;

			uint channel;
			uint bpp;
			uint pitch;
			uint data_size;

			FLAME_GRAPHICS_EXPORTS Imageview* default_view() const;

			FLAME_GRAPHICS_EXPORTS void change_layout(ImageLayout from, ImageLayout to);
			FLAME_GRAPHICS_EXPORTS void clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color);

			// accepted formats for get/set pixels: Format_R8G8B8A8_UNORM, Format_R16G16B16A16_UNORM
			FLAME_GRAPHICS_EXPORTS void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst);
			FLAME_GRAPHICS_EXPORTS void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src);

			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data = nullptr, bool create_defalut_view = true);
			FLAME_GRAPHICS_EXPORTS static Image* create_from_bitmap(Device* d, Bitmap* bmp, ImageUsageFlags extra_usage = 0, bool create_defalut_view = true); // default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static Image* create_from_file(Device* d, const wchar_t* filename, ImageUsageFlags extra_usage = 0, bool create_defalut_view = true); // default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static void save_to_png(Image *i, const wchar_t* filename);
			FLAME_GRAPHICS_EXPORTS static Image* create_from_native(Device* d, Format format, const Vec2u& size, uint level, uint layer, void* native, bool create_defalut_view = true);

			FLAME_GRAPHICS_EXPORTS static void destroy(Image* image);
		};

		struct Imageview
		{
			ImageviewType type;
			uint base_level;
			uint level_count;
			uint base_layer;
			uint layer_count;
			Swizzle swizzle_r;
			Swizzle swizzle_g;
			Swizzle swizzle_b;
			Swizzle swizzle_a;

			FLAME_GRAPHICS_EXPORTS Image* image() const;

			FLAME_GRAPHICS_EXPORTS static Imageview* create(Image* i, ImageviewType type = Imageview2D, uint base_level = 0, uint level_count = 1, uint base_layer = 0, uint layer_count = 1, 
				Swizzle swizzle_r = SwizzleIdentity, Swizzle swizzle_g = SwizzleIdentity, Swizzle swizzle_b = SwizzleIdentity, Swizzle swizzle_a = SwizzleIdentity);
			FLAME_GRAPHICS_EXPORTS static void destroy(Imageview* v);
		};

		struct Sampler
		{
			FLAME_GRAPHICS_EXPORTS static Sampler* get_default(Filter filter);
			FLAME_GRAPHICS_EXPORTS static void set_default(Sampler* nearest, Sampler* linear);
			FLAME_GRAPHICS_EXPORTS static Sampler* create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
			FLAME_GRAPHICS_EXPORTS static void destroy(Sampler* s);
		};

		struct Atlas : Object
		{
			bool border;

			void* canvas_;
			uint canvas_slot_;

			struct Tile
			{
				const wchar_t* filename;
				uint id;
				Vec2i pos;
				Vec2i size;
				Vec2f uv0;
				Vec2f uv1;
			};

			Atlas() :
				Object("Atlas")
			{
			}

			FLAME_GRAPHICS_EXPORTS Imageview* imageview() const;
			FLAME_GRAPHICS_EXPORTS uint tile_count() const;
			FLAME_GRAPHICS_EXPORTS const Tile& tile(uint idx) const;

			int find_tile(uint id) const
			{
				auto count = tile_count();
				for (auto i = 0; i < count; i++)
				{
					if (tile(i).id == id)
						return i;
				}
				return -1;
			}

			FLAME_GRAPHICS_EXPORTS static Atlas* load(Device* d, const wchar_t* filename);
			FLAME_GRAPHICS_EXPORTS static void destroy(Atlas* a);
		};
	}
}

