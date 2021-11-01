#pragma once

#include "imgui.h"

namespace flame
{
	struct sImguiPrivate : sImgui
	{
		graphics::Window* window = nullptr;

		void setup(graphics::Window* window) override;

		void on_added() override;

		void render(uint tar_idx, graphics::CommandBuffer* cb);
		void update() override;
	};
}
