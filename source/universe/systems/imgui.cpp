#include "../../foundation/window.h"
#include "../../graphics/buffer.h"
#include "../../graphics/image.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/shader.h"
#include "../../graphics/command.h"
#include "../../graphics/swapchain.h"
#include "../../graphics/window.h"
#include "../world_private.h"
#include "../components/imgui_private.h"
#include "imgui_private.h"

namespace flame
{
	using namespace graphics;

	graphics::Image* sImguiPrivate::set_render_target(graphics::Image* old, const uvec2& new_size)
	{
		if (old)
		{
			auto found = false;
			for (auto it = render_tars.begin(); it != render_tars.end(); it++)
			{
				if (it->get() == old)
				{
					Queue::get(nullptr)->wait_idle();
					render_tars.erase(it);
					found = true;
					break;
				}
			}
			assert(found);
		}

		if (new_size.x == 0 || new_size.y == 0)
			return nullptr;
		auto img = graphics::Image::create(nullptr, graphics::Format_B8G8R8A8_UNORM, new_size, 1, 1, graphics::SampleCount_1, graphics::ImageUsageTransferDst | graphics::ImageUsageAttachment | graphics::ImageUsageSampled);
		img->clear(ImageLayoutUndefined, ImageLayoutAttachment, cvec4(255));
		render_tars.emplace_back(img);
		return img;
	}

	sImgui* sImgui::create(void* parms)
	{
		return new sImguiPrivate();
	}
}
