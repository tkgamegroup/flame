#include <flame/universe/components/node.h>
#include "view_scene.h"
#include "selection.h"

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
		graphics::Queue::get(nullptr)->wait_idle();
		render_tar.reset(graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, uvec2(size), 1, 1, 
			graphics::SampleCount_1, graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		render_tar->change_layout(graphics::ImageLayoutShaderReadOnly);
		auto iv = render_tar->get_view();
		app.node_renderer->set_targets( { &iv, 1 }, graphics::ImageLayoutShaderReadOnly );
	}
	if (render_tar)
	{
		ImGui::Image(render_tar.get(), size);
		if (ImGui::IsItemHovered())
		{
			if (!camera_node)
			{
				if (app.e_prefab)
					camera_node = app.e_prefab->find_child("[Editor]")->get_component_i<cNodeT>(0);
			}
			if (camera_node)
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
				{
					auto disp = (vec2)ImGui::GetIO().MouseDelta;
					if (disp.x != 0.f || disp.y != 0.f)
					{
						if (ImGui::GetIO().KeyShift)
						{

						}
						else
						{
							disp /= vec2(render_tar->size);
							disp *= -180.f;
							camera_node->add_eul(vec3(disp, 0.f));
						}
					}
				}
				if (ImGui::IsKeyDown(Keyboard_W))
				{
					camera_node->add_pos(-camera_node->g_rot[2] * 0.2f);
					app.render_frames += 30;
				}
				if (ImGui::IsKeyDown(Keyboard_S))
				{
					camera_node->add_pos(+camera_node->g_rot[2] * 0.2f);
					app.render_frames += 30;
				}
				if (ImGui::IsKeyDown(Keyboard_A))
				{
					camera_node->add_pos(-camera_node->g_rot[0] * 0.2f);
					app.render_frames += 30;
				}
				if (ImGui::IsKeyDown(Keyboard_D))
				{
					camera_node->add_pos(+camera_node->g_rot[0] * 0.2f);
					app.render_frames += 30;
				}
				if (ImGui::IsKeyDown(Keyboard_F))
				{
					if (selection.type == Selection::tEntity)
					{
						if (auto node = selection.entity->get_component_i<cNodeT>(0); node)
							camera_node->set_pos(node->g_pos + camera_node->g_rot[2] * camera_zoom);
					}
				}
				if (auto scroll = ImGui::GetIO().MouseWheel; scroll != 0.f)
				{

				}
			}
		}
	}
}
