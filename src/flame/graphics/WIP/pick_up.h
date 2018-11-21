#pragma once

#include <memory>
#include <functional>

namespace flame
{
	struct Texture;
	struct Framebuffer;
	struct CommandBuffer;

	extern Texture *pick_up_image;
	extern Texture *pick_up_depth_image;
	extern std::shared_ptr<Framebuffer> pick_up_fb;

	unsigned int pick_up(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback);

	void init_pick_up();
}
