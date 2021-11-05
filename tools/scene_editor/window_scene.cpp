#include "window_scene.h"

WindowScene window_scene;

WindowScene::WindowScene() :
	Window("Scene")
{
}

void WindowScene::on_draw()
{
	auto size = ImGui::GetContentRegionAvail();
	if (!render_tar || render_tar->get_size() != uvec2(size.x, size.y))
	{
		render_tar = app.s_imgui->set_render_target(render_tar, uvec2(size.x, size.y));
		if (render_tar)
		{
			auto iv = render_tar->get_view();
			app.s_renderer->set_targets(1, &iv);
		}
		else
			app.s_renderer->set_targets(0, nullptr);
	}
	if (render_tar)
		ImGui::Image(render_tar, size);
}
