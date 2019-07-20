#pragma once

#include <flame/graphics/renderpass.h>
#include "graphics_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate *d;
			std::vector<AttachmentInfo> attachments;
			std::vector<SubpassInfo> subpasses;
#if defined(FLAME_VULKAN)
			VkRenderPass v;
#endif
			RenderpassPrivate(Device *d, const RenderpassInfo& info);
			~RenderpassPrivate();
		};

		struct ClearvaluesPrivate : Clearvalues
		{
			RenderpassPrivate* renderpass;

#if defined(FLAME_VULKAN)
			std::vector<VkClearValue> v;
#elif defined(FLAME_D3D12)
			std::vector<Vec4f> v;
#endif

			ClearvaluesPrivate(Renderpass *r);
			~ClearvaluesPrivate();

			void set(uint idx, const Vec4c& col);
		};

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* d;
			RenderpassPrivate* renderpass;
			Vec2u image_size;
#if defined(FLAME_VULKAN)
			VkFramebuffer v;
#elif defined(FLAME_D3D12)

#endif
			FramebufferPrivate(Device* d, const FramebufferInfo& info);
			~FramebufferPrivate();
		};
	}
}
