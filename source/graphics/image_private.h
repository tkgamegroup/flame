#pragma once

#include "image.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct ImagePrivate : Image
		{
			ImageUsageFlags usage;
			bool is_cube = false;
			bool srgb = false;

			VkImage vk_image = 0;
			VkDeviceMemory vk_memory = 0;
			std::map<uint64, std::unique_ptr<ImageViewPrivate>> views;
			std::map<uint64, std::unique_ptr<DescriptorSetPrivate>> read_dss;
			std::map<uint64, std::unique_ptr<FramebufferPrivate>> write_fbs;
			uint data_size;

			uint ref = 0;

			void initialize();
			~ImagePrivate();

			void get_data(uint level, uint layer);
			vec4 get_pixel(int x, int y, uint level, uint layer);
			void set_pixel(int x, int y, uint level, uint layer, const vec4& v);

			ImageViewPtr get_view(const ImageSub& sub = {}, const ImageSwizzle& swizzle = {}) override;
			DescriptorSetPtr get_shader_read_src(uint base_level = 0, uint base_layer = 0, SamplerPtr sp = nullptr) override;
			FramebufferPtr get_shader_write_dst(uint base_level = 0, uint base_layer = 0, AttachmentLoadOp load_op = AttachmentLoadDontCare) override;

			void change_layout(ImageLayout dst_layout) override;
			void clear(const vec4& color, ImageLayout dst_layout) override;

			vec4 linear_sample(const vec2& uv, uint level, uint layer) override;

			void save(const std::filesystem::path& filename) override;

			static ImagePtr create(DevicePtr device, Format format, const uvec2& size, VkImage native);
		};

		struct ImageViewPrivate : ImageView
		{
			VkImageView vk_image_view;

			~ImageViewPrivate();
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
			VkSampler vk_sampler;

			~SamplerPrivate();
		};

		struct ImageAtlasPrivate : ImageAtlas
		{
			~ImageAtlasPrivate();
		};

		extern std::vector<std::unique_ptr<ImageT>> loaded_images;
		extern std::vector<std::unique_ptr<SamplerT>> samplers;
	}
}

