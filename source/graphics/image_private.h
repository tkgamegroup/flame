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
			std::vector<Vec2u> sizes;
			uint level;
			uint layer;
			SampleCount sample_count;

			VkDeviceMemory vk_memory = 0;
			VkImage vk_image = 0;
			std::vector<std::unique_ptr<ImageViewPrivate>> default_views;

			void init(const Vec2u& size);
			void build_default_views();
			ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data = nullptr);
			ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, void* native);
			~ImagePrivate();

			void release() override { delete this; }

			Format get_format() const override { return format; }
			Vec2u get_size(uint lv) const override { return sizes[lv]; }
			uint get_level() const override { return level; }
			uint get_layer() const override { return layer; }
			SampleCount get_sample_count() const override { return sample_count; }

			ImageView* get_default_view(uint level) const override { return (ImageView*)default_views[level].get(); }

			void clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color) override;

			void get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst) override;
			void set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src) override;

			static ImagePrivate* create(DevicePrivate* d, Bitmap* bmp, ImageUsageFlags extra_usage = ImageUsageNone);
			static ImagePrivate* create(DevicePrivate* d, const std::filesystem::path& filename, ImageUsageFlags extra_usage = ImageUsageNone);
		};

		struct ImageViewPrivate : ImageView
		{
			DevicePrivate* device;
			ImagePrivate* image;

			ImageViewType type;
			ImageSubresource subresource;
			ImageSwizzle swizzle;

			VkImageView vk_image_view;

			ImageViewPrivate(ImagePrivate* image, ImageViewType type = ImageView2D, const ImageSubresource& subresource = {}, const ImageSwizzle& swizzle = {});
			~ImageViewPrivate();

			void release() override { delete this; }

			ImageViewType get_type() const override { return type; }
			ImageSubresource get_subresource() const override { return subresource; }
			ImageSwizzle get_swizzle() const override { return swizzle; }

			Image* get_image() const override { return image; }
		};

		inline ImageAspectFlags aspect_from_format(Format fmt)
		{
			if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				return ImageAspectColor;
			if (fmt >= Format_Depth_Begin && fmt <= Format_Depth_End)
			{
				auto a = (int)ImageAspectDepth;
				if (fmt >= Format_DepthStencil_Begin && fmt <= Format_DepthStencil_End)
					a |= ImageAspectStencil;
				return (ImageAspectFlags)a;
			}
			return ImageAspectColor;
		}

		struct SamplerPrivate : Sampler
		{
			DevicePrivate* device;
			VkSampler vk_sampler;

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

			uint get_index() const override { return index; }
			const char* get_name() const override { return name.c_str(); }
			Vec2i get_pos() const override { return pos; }
			Vec2i get_size() const override { return size; }
			Vec4f get_uv() const override { return uv; }
		};

		struct ImageAtlasBridge : ImageAtlas
		{
			ImageTile* get_tile(uint id) const override;
			ImageTile* find_tile(const char* name) const override;
		};

		struct ImageAtlasPrivate : ImageAtlasBridge
		{
			bool border = false;

			ImagePrivate* image;
			std::vector<std::unique_ptr<ImageTilePrivate>> tiles;

			ImageAtlasPrivate(DevicePrivate* d, const std::wstring& atlas_filename);
			~ImageAtlasPrivate();

			void release() override { delete this; }

			bool get_border() const override { return border; }

			ImageTilePrivate* get_tile(uint id) const override { return tiles[id].get(); }
			ImageTilePrivate* find_tile(const std::string& name) const;

			Image* get_image() const override { return image; }
		};

		inline ImageTile* ImageAtlasBridge::get_tile(uint id) const
		{
			return ((ImageAtlasPrivate*)this)->get_tile(id);
		}

		inline ImageTile* ImageAtlasBridge::find_tile(const char* name) const
		{
			return ((ImageAtlasPrivate*)this)->find_tile(name);
		}
	}
}

