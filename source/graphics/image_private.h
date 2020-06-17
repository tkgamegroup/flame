#pragma once

#include <flame/graphics/image.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct ImageviewPrivate;

		struct ImagePrivate : Image
		{
			DevicePrivate* d;
			
			Format format;
			Vec2u size;
			uint level;
			uint layer;
			SampleCount sample_count;

#if defined(FLAME_VULKAN)
			VkDeviceMemory m;
			VkImage v;
#elif defined(FLAME_D3D12)
			ID3D12Resource* v;
#endif
			std::unique_ptr<ImageviewPrivate> dv;

			ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, bool default_view = true);
			ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, void* native, bool default_view = true);
			~ImagePrivate();

			void release() override;

			Format get_format() const override;
			Vec2u get_size() const override;
			uint get_level() const override;
			uint get_layer() const override;
			SampleCount get_sample_count() const override;

			Imageview* get_default_view() const override;

			void change_layout(ImageLayout from, ImageLayout to) override;
			void clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color) override;

			void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst) override;
			void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src) override;
		};

		struct ImageviewPrivate : Imageview
		{
			DevicePrivate* d;
			ImagePrivate* image;

			ImageviewType type;
			uint base_level;
			uint level_count;
			uint base_layer;
			uint layer_count;
			Swizzle swizzle_r;
			Swizzle swizzle_g;
			Swizzle swizzle_b;
			Swizzle swizzle_a;

#if defined(FLAME_VULKAN)
			VkImageView v;
#elif defined(FLAME_D3D12)
			ID3D12DescriptorHeap* v;
#endif
			int ref_count;

			ImageviewPrivate(ImagePrivate* image, ImageviewType type = Imageview2D, uint base_level = 0, uint level_count = 1, uint base_layer = 0, uint layer_count = 1,
				Swizzle swizzle_r = SwizzleIdentity, Swizzle swizzle_g = SwizzleIdentity, Swizzle swizzle_b = SwizzleIdentity, Swizzle swizzle_a = SwizzleIdentity);
			~ImageviewPrivate();

			void release() override;

			ImageviewType get_type() const override;
			uint get_base_level() const override;
			uint get_level_count() const override;
			uint get_base_layer() const override;
			uint get_layer_count() const override;
			Swizzle get_swizzle_r() const override;
			Swizzle get_swizzle_g() const override;
			Swizzle get_swizzle_b() const override;
			Swizzle get_swizzle_a() const override;

			Image* get_image() const override;
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
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkSampler v;
#elif defined(FLAME_D3D12)

#endif
			SamplerPrivate(DevicePrivate* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
			~SamplerPrivate();

			void release() override;
		};

		struct ImageTilePrivate : ImageTile
		{
			uint index;
			std::wstring filename;
			uint id;
			Vec2i pos;
			Vec2i size;
			Vec4f uv;

			uint get_index() const override;
			const wchar_t* get_filename() const override;
			uint get_id() const override;
			Vec2i get_pos() const override;
			Vec2i get_size() const override;
			Vec4f get_uv() const override;
		};

		struct ImageAtlasPrivate : ImageAtlas
		{
			bool border;

			int slot;

			ImagePrivate* image;
			std::vector<std::unique_ptr<ImageTilePrivate>> tiles;

			ImageAtlasPrivate(DevicePrivate* d, const std::wstring& atlas_filename);
			~ImageAtlasPrivate();

			void release() override;

			bool get_border() const override;

			uint get_tiles_count() const override;
			ImageTile* get_tile(uint idx) const override;
			ImageTile* find_tile(uint id) const override;
		};
	}
}

