#pragma once

#include "image.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct ImagePrivate : Image
		{
			VkImage vk_image = 0;
			VkDeviceMemory vk_memory = 0;
			ID3D12Resource* d12_resource = nullptr;
			std::map<uint64, std::unique_ptr<ImageViewPrivate>> views;
			std::map<uint64, std::unique_ptr<DescriptorSetPrivate>> read_dss;
			std::map<uint64, std::unique_ptr<FramebufferPrivate>> write_fbs;

			ImagePrivate();
			~ImagePrivate();
			void initialize();

			ImageLayout get_layout(const ImageSub& sub = {}) override;
			ImageViewPtr get_view(const ImageSub& sub = {}, const ImageSwizzle& swizzle = {}, bool cube = false) override;
			DescriptorSetPtr get_shader_read_src(uint base_level = 0, uint base_layer = 0, SamplerPtr sp = nullptr, const ImageSwizzle& swizzle = {}) override;
			FramebufferPtr get_shader_write_dst(uint base_level = 0, uint base_layer = 0, AttachmentLoadOp load_op = AttachmentLoadDontCare) override;

			void change_layout(ImageLayout dst_layout) override;
			void clear(const vec4& color, ImageLayout dst_layout) override;

			vec4 get_pixel(int x, int y, uint level, uint layer) override;
			void stage_surface_data(uint level, uint layer);
			vec4 get_staging_pixel(int x, int y, uint level, uint layer) override;
			void set_staging_pixel(int x, int y, uint level, uint layer, const vec4& v) override;
			void upload_staging_pixels(int x, int y, int w, int h, uint level, uint layer) override;
			void clear_staging_pixels() override;

			vec4 linear_sample_staging_pixels(const vec2& uv, uint level, uint layer) override;

			void save(const std::filesystem::path& filename, bool compress) override;

			static ImagePtr create(DevicePtr device, Format format, const uvec3& extent, VkImage native);
		};

		struct ImageViewPrivate : ImageView
		{
			VkImageView vk_image_view;

			~ImageViewPrivate();

			DescriptorSetPtr get_shader_read_src(SamplerPtr sp) override;
		};

		inline ImageAspectFlags aspect_from_format(Format fmt)
		{
			if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				return ImageAspectColor;
			if (fmt >= Format_Stencil_Begin && fmt <= Format_Stencil_End)
				return ImageAspectStencil;
			if (fmt >= Format_DepthStencil_Begin && fmt <= Format_DepthStencil_End)
				return ImageAspectDepth | ImageAspectStencil;
			if (fmt >= Format_Depth_Begin && fmt <= Format_Depth_End)
				return ImageAspectDepth;
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

		extern std::vector<ImagePtr> images;
		extern std::vector<std::pair<std::unique_ptr<ImageT>, ImageConfig>> loaded_images;
		extern std::vector<std::unique_ptr<SamplerT>> shared_samplers;
		extern std::vector<std::unique_ptr<ImageAtlasT>> loaded_image_atlases;
	}
}

