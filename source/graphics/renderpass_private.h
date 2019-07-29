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
			DevicePrivate* d;
			std::vector<AttachmentInfo> attachments;
			std::vector<SubpassInfo> subpasses;
#if defined(FLAME_VULKAN)
			VkRenderPass v;
#endif
			RenderpassPrivate(Device* d, const std::vector<void*>& attachments, const std::vector<void*>& subpasses, const std::vector<Vec<2, uint>>& dependencies);
			~RenderpassPrivate();
		};

		struct ClearvaluesPrivate : Clearvalues
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
			FramebufferPrivate(Device* d, Renderpass* rp, const std::vector<void*>& views);
			~FramebufferPrivate();
		};

		struct RenderpassAndFramebufferPrivate : RenderpassAndFramebuffer
		{
			RenderpassPrivate* rp;
			std::vector<Imageview*> created_views;
			std::vector<void*> fbs;
			ClearvaluesPrivate* cv;

			RenderpassAndFramebufferPrivate(Device* d, const std::vector<void*>& passes);
			~RenderpassAndFramebufferPrivate();
		};
	}
}
