#include <flame/global.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/framebuffer.h>
#include <flame/engine/graphics/display_layer.h>
#include <flame/engine/ui/ui.h>

namespace flame
{
	bool DisplayLayer::on_message(Object *sender, Message msg)
	{
		switch (msg)
		{
			case MessageResolutionChange:
				create();
				return true;
		}
	}

	DisplayLayer::DisplayLayer(bool _enable_depth) :
		enable_depth(_enable_depth)
	{
		follow_to(&resolution);
		create();
	}

	DisplayLayer::~DisplayLayer()
	{
		ui::decrease_texture_ref(image.get());
	}

	void DisplayLayer::create()
	{
		if (image && image->get_cx() == resolution.x() && image->get_cy() == resolution.y())
			return;

		image = std::make_shared<Texture>(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_R8G8B8A8_UNORM, 0);
		ui::increase_texture_ref(image.get());
		if (enable_depth)
		{
			depth_image = std::make_unique<Texture>(TextureTypeAttachment, resolution.x(), resolution.y(), VK_FORMAT_D16_UNORM, 0);
			VkImageView views[] = {
				image->get_view(),
				depth_image->get_view()
			};
			renderpass = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, true)
				.add_attachment(VK_FORMAT_D16_UNORM, false)
				.add_subpass({0}, 1)
			);
			framebuffer = get_framebuffer(resolution.x(), resolution.y(), renderpass.get(), TK_ARRAYSIZE(views), views);
		}
		else
		{
			renderpass = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, false)
				.add_subpass({0}, -1)
			);
			framebuffer = get_framebuffer(image.get(), renderpass.get());
		}
	}
}
