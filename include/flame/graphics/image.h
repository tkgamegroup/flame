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

			// [0, level-1]: view of that level
			// [level]: view of all levels
			// [>level]: auto released views
			virtual ImageView* get_view(uint idx = 0) const = 0;

			virtual void clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color) = 0;

			// accepted formats for get/set pixels: Format_R8G8B8A8_UNORM, Format_R16G16B16A16_UNORM
			virtual void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst) = 0;
			virtual void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src) = 0;

			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data = nullptr, bool is_cube = false);
			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, Bitmap* bmp, ImageUsageFlags extra_usage = ImageUsageNone); // default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, const wchar_t* filename, ImageUsageFlags extra_usage = ImageUsageNone); // default usage: ShaderSampled, TransferDst

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
			
			FLAME_GRAPHICS_EXPORTS static Sampler* create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates, bool clamp_to_edge = true);
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

			FLAME_GRAPHICS_EXPORTS static ImageAtlas* create(Device* d, const wchar_t* filename);
		};
	}
}

