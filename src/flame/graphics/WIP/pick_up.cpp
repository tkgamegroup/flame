#include <flame/global.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/graphics/framebuffer.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/graphics/pick_up.h>

namespace flame
{
	Texture *pick_up_image = nullptr;
	Texture *pick_up_depth_image = nullptr;
	static std::shared_ptr<RenderPass> renderpass;
	std::shared_ptr<Framebuffer> pick_up_fb;

	unsigned int pick_up(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback)
	{
		if (x < 0 || y < 0 || x > pick_up_image->get_cx() || y > pick_up_image->get_cy())
			return 0;

		auto cb = begin_once_command_buffer();
		cb->begin_renderpass(renderpass.get(), pick_up_fb.get());
		drawCallback(cb);
		cb->end_renderpass();
		end_once_command_buffer(cb);

		{
			VkBufferImageCopy r = {};
			r.imageOffset.x = x;
			r.imageOffset.y = y;
			r.imageExtent.width = 1;
			r.imageExtent.height = 1;
			r.imageExtent.depth = 1;
			r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			r.imageSubresource.layerCount = 1;

			auto cb = flame::begin_once_command_buffer();
			pick_up_image->transition_layout(cb, pick_up_image->layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			vkCmdCopyImageToBuffer(cb->v, pick_up_image->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, defalut_staging_buffer->v, 1, &r);
			pick_up_image->transition_layout(cb, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pick_up_image->layout);
			flame::end_once_command_buffer(cb);
		}

		defalut_staging_buffer->map(0, 4);
		auto pixel = (unsigned char*)defalut_staging_buffer->mapped;
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);
		defalut_staging_buffer->unmap();

		return index;
	}

	void init_pick_up() 
	{
		pick_up_image = new Texture(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		pick_up_depth_image = new Texture(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_D16_UNORM, 0);

		VkImageView views[] = {
			pick_up_image->get_view(),
			pick_up_depth_image->get_view()
		};

		renderpass = get_renderpass(RenderPassInfo()
			.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, true)
			.add_attachment(VK_FORMAT_D16_UNORM, true)
			.add_subpass({ 0 }, 1)
		);

		pick_up_fb = get_framebuffer(resolution.x(), resolution.y(), renderpass.get(), TK_ARRAYSIZE(views), views);
	}
}
