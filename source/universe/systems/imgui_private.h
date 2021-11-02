#pragma once

#include "imgui.h"

namespace flame
{
	struct Vertex
	{
		vec2 pos;
		vec2 uv;
		cvec4 col;
	};

	struct RenderData;

	struct sImguiPrivate : sImgui
	{
		graphics::Window* window = nullptr;

		graphics::Renderpass* rp_bgra8;
		graphics::Renderpass* rp_bgra8l;
		graphics::Renderpass* rp_bgra8c;

		std::vector<graphics::Image*> img_tars;
		std::vector<UniPtr<graphics::Framebuffer>> fb_tars;
		uvec2 tar_sz;

		RenderData* _rd;

		sImguiPrivate();
		~sImguiPrivate();
		void setup(graphics::Window* window) override;

		void on_added() override;

		void set_targets(const std::vector<graphics::ImageView*>& views);

		void render(uint tar_idx, graphics::CommandBuffer* cb);
		void update() override;
	};
}
