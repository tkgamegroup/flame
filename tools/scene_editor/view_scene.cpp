#include "view_scene.h"

View_Scene view_scene;

View_Scene::View_Scene() :
	View("Scene")
{
}

void View_Scene::open_scene(const std::filesystem::path& path)
{

}

void View_Scene::open_prefab(const std::filesystem::path& path)
{
	//if (e_prefab)
	//	e_prefab->get_parent()->remove_child(e_prefab);
	//e_prefab = Entity::create();
	//e_prefab->load(path.c_str());
	//app.root->add_child(e_prefab);
}

void View_Scene::on_draw()
{
	auto size = ImGui::GetContentRegionAvail();
	auto usize = uvec2(size.x, size.y);
	if (!render_tar || render_tar->sizes[0] != usize)
	{
		//render_tar = app.s_imgui->set_render_target(render_tar, usize);
		//if (render_tar)
		//{
		//	auto iv = render_tar->get_view();
		//	app.s_renderer->set_targets(1, &iv);
		//}
		//else
		//	app.s_renderer->set_targets(0, nullptr);
	}
	if (render_tar)
		ImGui::Image(render_tar, size);
}
