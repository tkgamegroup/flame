#include "view_scene.h"

View_Scene view_scene;

View_Scene::View_Scene() :
	View("Scene")
{
}

void View_Scene::on_draw()
{
	auto size = vec2(ImGui::GetContentRegionAvail());
	if (!render_tar || vec2(render_tar->size) != size)
	{
		render_tar = graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, uvec2(size), 1, 1, 
			graphics::SampleCount_1, graphics::ImageUsageAttachment | graphics::ImageUsageSampled);
		render_tar->change_layout(graphics::ImageLayoutShaderReadOnly);
		auto iv = render_tar->get_view();
		app.node_renderer->set_targets( { &iv, 1 }, graphics::ImageLayoutShaderReadOnly );
	}
	if (render_tar)
		ImGui::Image(render_tar, size);
}
