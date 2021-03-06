#pragma once

#include "renderpass.h"
#include "graphics_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate* device;

			std::filesystem::path filename;

			std::vector<RenderpassAttachmentInfo> attachments;
			std::vector<RenderpassSubpassInfo> subpasses;

			VkRenderPass vk_renderpass;

			RenderpassPrivate(DevicePrivate* device, std::span<const RenderpassAttachmentInfo> attachments, 
				std::span<const RenderpassSubpassInfo> subpasses, std::span<const uvec2> dependencies = {});
			~RenderpassPrivate();

			void release() override { delete this; }

			uint get_attachments_count() const override { return attachments.size(); }
			void get_attachment_info(uint idx, RenderpassAttachmentInfo* dst) const override { *dst = attachments[idx]; }
			uint get_subpasses_count() const override { return subpasses.size(); }
			void get_subpass_info(uint idx, RenderpassSubpassInfo* dst) const override { *dst = subpasses[idx]; }

			static RenderpassPrivate* get(DevicePrivate* device, const std::filesystem::path& filename);
		};

		extern std::vector<RenderpassPrivate*> __renderpasses;

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* device;

			RenderpassPtr renderpass;
			std::vector<ImageViewPrivate*> views;
			VkFramebuffer vk_framebuffer;

			FramebufferPrivate(DevicePrivate* device, RenderpassPrivate* rp, std::span<ImageViewPrivate*> views);
			~FramebufferPrivate();

			void release() override { delete this; }

			RenderpassPtr get_renderpass() const override { return renderpass; }
			uint get_views_count() const override { return views.size(); }
			ImageViewPtr get_view(uint idx) const override { return views[idx]; }
		};
	}
}
