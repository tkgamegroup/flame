#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	struct Bitmap;

	namespace graphics
	{
		struct Device;
		struct ImageView;

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
			virtual Vec2u get_size(uint lv = 0) const = 0;
			virtual uint get_level() const = 0;
			virtual uint get_layer() const = 0;
			virtual SampleCount get_sample_count() const = 0;

			virtual const wchar_t* get_filename() const = 0;

			// [0, level-1]: view of that level
			// [level]: view of all levels
			// [>level]: auto released views
			virtual ImageView* get_view(uint idx = 0) const = 0;

			FLAME_GRAPHICS_EXPORTS static Image* create(Device* device, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, bool is_cube = false);
			FLAME_GRAPHICS_EXPORTS static Image* create(Device* device, Bitmap* bmp);
			FLAME_GRAPHICS_EXPORTS static Image* create(Device* device, const wchar_t* filename, bool srgb, ImageUsageFlags additional_usage = ImageUsageNone);

		};

		struct ImageView
		{
			virtual void release() = 0;

			virtual Image* get_image() const = 0;

			virtual ImageViewType get_type() const = 0;
			virtual ImageSubresource get_subresource() const = 0;
			virtual ImageSwizzle get_swizzle() const = 0;

			FLAME_GRAPHICS_EXPORTS static ImageView* create(Image* image, bool auto_released, ImageViewType type = ImageView2D, const ImageSubresource& subresource = {}, const ImageSwizzle& swizzle = {});
		};

		struct Sampler
		{
			virtual void release() = 0;
			
			FLAME_GRAPHICS_EXPORTS static Sampler* create(Device* device, Filter mag_filter, Filter min_filter, AddressMode address_mode = AddressClampToEdge, bool unnormalized_coordinates = true);
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

			virtual ImageTile* get_tile(uint id) const = 0;
			virtual ImageTile* find_tile(const char* name) const = 0;

			virtual Image* get_image() const = 0;

			FLAME_GRAPHICS_EXPORTS static ImageAtlas* create(Device* device, const wchar_t* filename);
		};
	}
}

