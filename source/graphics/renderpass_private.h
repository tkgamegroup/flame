#pragma once

#include <flame/graphics/renderpass.h>
#include "graphics_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct RenderpassAttachmentPrivate : RenderpassAttachment
		{
			uint index;
			Format format;
			bool clear;
			SampleCount sample_count;
		};

		struct RenderpassSubpassPrivate : RenderpassSubpass
		{
			std::vector<RenderpassAttachmentPrivate*> color_attachments;
			std::vector<RenderpassAttachmentPrivate*> resolve_attachments;
			RenderpassAttachmentPrivate* depth_attachment;
		};

		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate* d;

			std::vector<std::unique_ptr<RenderpassAttachmentPrivate>> attachments;
			std::vector<std::unique_ptr<RenderpassSubpassPrivate>> subpasses;

#if defined(FLAME_VULKAN)
			VkRenderPass v;
#endif
			RenderpassPrivate(DevicePrivate* d, const std::span<RenderpassAttachmentInfo>& attachments, const std::span<RenderpassSubpassInfo>& subpasses, const std::span<Vec2u>& dependencies);
			~RenderpassPrivate();

			void release() override;
		};

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* d;

			RenderpassPrivate* rp;
			std::vector<ImageviewPrivate*> views;
#if defined(FLAME_VULKAN)
			VkFramebuffer v;
#elif defined(FLAME_D3D12)

#endif
			FramebufferPrivate(DevicePrivate* d, RenderpassPrivate* rp, const std::span<ImageviewPrivate*>& views);
			~FramebufferPrivate();

			void release() override;
		};
	}
}
