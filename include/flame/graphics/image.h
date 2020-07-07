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
			virtual void release() = 0;

			virtual Format get_format() const = 0;
			virtual Vec2u get_size() const = 0;
			virtual uint get_level() const = 0;
			virtual uint get_layer() const = 0;
			virtual SampleCount get_sample_count() const = 0;

			virtual Imageview* get_default_view() const = 0;

			virtual void change_layout(ImageLayout from, ImageLayout to) = 0;
			virtual void clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color) = 0;

			// accepted formats for get/set pixels: Format_R8G8B8A8_UNORM, Format_R16G16B16A16_UNORM
			virtual void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst) = 0;
			virtual void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src) = 0;

			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data = nullptr, bool create_defalut_view = true);
			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, Bitmap* bmp, ImageUsageFlags extra_usage = 0, bool create_defalut_view = true); // default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, const wchar_t* filename, ImageUsageFlags extra_usage = 0, bool create_default_view = true); // default usage: ShaderSampled, TransferDst

		};

		struct Imageview
		{
			virtual void release() = 0;

			virtual Image* get_image() const = 0;

			virtual ImageviewType get_type() const = 0;
			virtual uint get_base_level() const = 0;
			virtual uint get_level_count() const = 0;
			virtual uint get_base_layer() const = 0;
			virtual uint get_layer_count() const = 0;
			virtual Swizzle get_swizzle_r() const = 0;
			virtual Swizzle get_swizzle_g() const = 0;
			virtual Swizzle get_swizzle_b() const = 0;
			virtual Swizzle get_swizzle_a() const = 0;

			FLAME_GRAPHICS_EXPORTS static Imageview* create(Image* i, ImageviewType type = Imageview2D, uint base_level = 0, uint level_count = 1, uint base_layer = 0, uint layer_count = 1, 
				Swizzle swizzle_r = SwizzleIdentity, Swizzle swizzle_g = SwizzleIdentity, Swizzle swizzle_b = SwizzleIdentity, Swizzle swizzle_a = SwizzleIdentity);
		};

		struct Sampler
		{
			virtual void release() = 0;
			
			FLAME_GRAPHICS_EXPORTS static Sampler* create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
		};

		struct ImageTile
		{
			virtual uint get_index() const = 0;
			virtual const char* get_name() const = 0;
			virtual Vec2i get_pos() const = 0;
			virtual Vec2i get_size() const = 0;
			virtual Vec4f get_uv() const = 0;
		};

		struct ImageAtlas
		{
			virtual void release() = 0;

			virtual bool get_border() const = 0;

			virtual ImageTile* find_tile(const char* name) const = 0;

			FLAME_GRAPHICS_EXPORTS static ImageAtlas* create(Device* d, const wchar_t* filename);
		};
	}
}

