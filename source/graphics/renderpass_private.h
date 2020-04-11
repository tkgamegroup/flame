#pragma once

#include <flame/graphics/renderpass.h>
#include "graphics_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct SubpassInfoPrivate : SubpassInfo
		{
			std::vector<uint> _color_attachments;
			std::vector<uint> _resolve_attachments;

			void operator=(const SubpassInfo& info)
			{
				_color_attachments.resize(info.color_attachment_count);
				for (auto i = 0; i < info.color_attachment_count; i++)
					_color_attachments[i] = info.color_attachments[i];
				_resolve_attachments.resize(info.resolve_attachment_count);
				for (auto i = 0; i < info.resolve_attachment_count; i++)
					_resolve_attachments[i] = info.resolve_attachments[i];
				color_attachment_count = info.color_attachment_count;
				color_attachments = _color_attachments.data();
				resolve_attachment_count = info.resolve_attachment_count;
				resolve_attachments = _resolve_attachments.data();
				depth_attachment = info.depth_attachment;
			}
		};

		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate* d;
			std::vector<AttachmentInfo> attachments;
			std::vector<SubpassInfoPrivate> subpasses;
#if defined(FLAME_VULKAN)
			VkRenderPass v;
#endif
			RenderpassPrivate(Device* d, uint attachment_count, const AttachmentInfo* attachments, uint subpass_count, const SubpassInfo* subpasses, uint dependency_count, const Vec2u* dependencies);
			~RenderpassPrivate();
		};

		struct ClearvaluesPrivate
		{
			RenderpassPrivate* rp;

#if defined(FLAME_VULKAN)
			std::vector<VkClearValue> v;
#elif defined(FLAME_D3D12)
			std::vector<Vec4f> v;
#endif

			ClearvaluesPrivate(Renderpass* rp);
			~ClearvaluesPrivate();

			void set(uint idx, const Vec4c& col);
		};

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* d;
			RenderpassPrivate* rp;
#if defined(FLAME_VULKAN)
			VkFramebuffer v;
#elif defined(FLAME_D3D12)

#endif
			FramebufferPrivate(Device* d, Renderpass* rp, uint view_count, Imageview* const* views);
			~FramebufferPrivate();
		};
	}
}
