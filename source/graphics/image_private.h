#pragma once

#include <flame/graphics/image.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct ImageViewPrivate;

		struct ImagePrivate : Image
		{
			DevicePrivate* device;
			
			Format format;
			Vec2u size;
			uint level;
			uint layer;
			SampleCount sample_count;

#if defined(FLAME_VULKAN)
			VkDeviceMemory vk_memory;
			VkImage vk_image;
#elif defined(FLAME_D3D12)
			ID3D12Resource* _v;
#endif
			std::unique_ptr<ImageViewPrivate> default_view;

			ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data = nullptr, bool default_view = true);
			ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, void* native, bool default_view = true);
			~ImagePrivate();

			void _change_layout(ImageLayout from, ImageLayout to);
			void _clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color);

			void _get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst);
			void _set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src);

			static ImagePrivate* _create(DevicePrivate* d, Bitmap* bmp, ImageUsageFlags extra_usage = 0, bool create_defalut_view = true);
			static ImagePrivate* _create(DevicePrivate* d, const std::filesystem::path& filename, ImageUsageFlags extra_usage = 0, bool create_defalut_view = true);

			void release() override { delete this; }

			Format get_format() const override { return _format; }
			Vec2u get_size() const override { return _size; }
			uint get_level() const override { return _level; }
			uint get_layer() const override { return _layer; }
			SampleCount get_sample_count() const override { return _sample_count; }

			ImageView* get_default_view() const override { return (ImageView*)_dv.get(); }

			void change_layout(ImageLayout from, ImageLayout to) override { _change_layout(from, to); }
			void clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color) override { _clear(current_layout, after_layout, color); }

			void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst) override { _get_pixels(offset, extent, dst); }
			void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src) override { _set_pixels(offset, extent, src); }
		};

		struct ImageViewPrivate : ImageView
		{
			DevicePrivate* device;
			ImagePrivate* image;

			ImageViewType type;
			uint base_level;
			uint level_count;
			uint base_layer;
			uint layer_count;
			Swizzle swizzle_r;
			Swizzle swizzle_g;
			Swizzle swizzle_b;
			Swizzle swizzle_a;

#if defined(FLAME_VULKAN)
			VkImageView vk_image_view;
#elif defined(FLAME_D3D12)
			ID3D12DescriptorHeap* _v;
#endif

			ImageViewPrivate(ImagePrivate* image, ImageViewType type = ImageView2D, uint base_level = 0, uint level_count = 1, uint base_layer = 0, uint layer_count = 1,
				Swizzle swizzle_r = SwizzleIdentity, Swizzle swizzle_g = SwizzleIdentity, Swizzle swizzle_b = SwizzleIdentity, Swizzle swizzle_a = SwizzleIdentity);
			~ImageViewPrivate();

			void release() override { delete this; }

			ImageViewType get_type() const override { return _type; }
			uint get_base_level() const override { return _base_level; }
			uint get_level_count() const override { return _level_count; }
			uint get_base_layer() const override { return _base_layer; }
			uint get_layer_count() const override { return _layer_count; }
			Swizzle get_swizzle_r() const override { return _swizzle_r; }
			Swizzle get_swizzle_g() const override { return _swizzle_g; }
			Swizzle get_swizzle_b() const override { return _swizzle_b; }
			Swizzle get_swizzle_a() const override { return _swizzle_a; }

			Image* get_image() const override { return _image; }
		};

		inline ImageAspectFlags aspect_from_format(Format fmt)
		{
			if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				return ImageAspectColor;
			if (fmt >= Format_Depth_Begin && fmt <= Format_Depth_End)
			{
				int a = ImageAspectDepth;
				if (fmt >= Format_DepthStencil_Begin && fmt <= Format_DepthStencil_End)
					a |= ImageAspectStencil;
				return a;
			}
			return ImageAspectColor;
		}

		struct SamplerPrivate : Sampler
		{
			DevicePrivate* device;
#if defined(FLAME_VULKAN)
			VkSampler vk_sampler;
#elif defined(FLAME_D3D12)

#endif
			SamplerPrivate(DevicePrivate* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
			~SamplerPrivate();

			void release() override { delete this; }
		};

		struct ImageTilePrivate : ImageTile
		{
			uint index;
			std::string name;
			Vec2i pos;
			Vec2i size;
			Vec4f uv;

			uint get_index() const override { return _index; }
			const char* get_name() const override { return _name.c_str(); }
			Vec2i get_pos() const override { return _pos; }
			Vec2i get_size() const override { return _size; }
			Vec4f get_uv() const override { return _uv; }
		};

		struct ImageAtlasPrivate : ImageAtlas
		{
			bool border = false;

			ImagePrivate* image;
			std::vector<std::unique_ptr<ImageTilePrivate>> tiles;

			ImageAtlasPrivate(DevicePrivate* d, const std::wstring& atlas_filename);
			~ImageAtlasPrivate();

			ImageTile* _find_tile(const std::string& name) const;

			void release() override { delete this; }

			bool get_border() const override { return _border; }

			ImageTile* find_tile(const char* name) const override { return _find_tile(name); }
		};
	}
}

