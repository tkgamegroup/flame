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
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		if (ImGui::IsItemHovered())
		{
			if (!camera_node)
			{
				if (app.e_prefab)
					camera_node = app.e_prefab->find_child("[Editor]")->get_component_i<cNodeT>(0);
			}
			if (camera_node)
			{
				auto get_tar = [&]() {
					return camera_node->g_pos - camera_node->g_rot[2] * camera_zoom;
				};

				auto& io = ImGui::GetIO();
				if (io.KeyAlt)
				{
					if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
					{
						disp /= vec2(render_tar->size);
						disp *= -180.f;
						if (io.MouseDown[ImGuiMouseButton_Left])
							camera_node->add_eul(vec3(disp, 0.f));
						else if (io.MouseDown[ImGuiMouseButton_Middle])
						{
							auto tar = get_tar();
							camera_node->add_eul(vec3(disp, 0.f));
							auto eul = camera_node->eul;
							auto rot = mat3(eulerAngleYXZ(radians(eul.x), radians(eul.y), radians(eul.z)));
							camera_node->set_pos(tar + rot[2] * camera_zoom);
						}
					}
				}
				if (io.KeyShift)
				{
					if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
					{
						disp /= vec2(render_tar->size);
						if (io.MouseDown[ImGuiMouseButton_Middle])
						{
							camera_node->add_pos((-camera_node->g_rot[0] * disp.x + 
								camera_node->g_rot[1] * disp.y) * camera_zoom);
						}
					}
				}
				if (io.KeysDown[Keyboard_W])
				{
					camera_node->add_pos(-camera_node->g_rot[2] * 0.2f);
					app.render_frames += 30;
				}
				if (io.KeysDown[Keyboard_S])
				{
					camera_node->add_pos(+camera_node->g_rot[2] * 0.2f);
					app.render_frames += 30;
				}
				if (io.KeysDown[Keyboard_A])
				{
					camera_node->add_pos(-camera_node->g_rot[0] * 0.2f);
					app.render_frames += 30;
				}
				if (io.KeysDown[Keyboard_D])
				{
					camera_node->add_pos(+camera_node->g_rot[0] * 0.2f);
					app.render_frames += 30;
				}
				if (io.KeysDown[Keyboard_F])
				{
					if (selection.type == Selection::tEntity)
					{
						if (auto node = selection.entity->get_component_i<cNodeT>(0); node)
							camera_node->set_pos(node->g_pos + camera_node->g_rot[2] * camera_zoom);
					}
				}
				if (auto scroll = io.MouseWheel; scroll != 0.f)
				{
					auto tar = get_tar();
					if (scroll < 0.f)
						camera_zoom = camera_zoom * 1.1f + 0.5f;
					else
						camera_zoom = max(0.f, camera_zoom / 1.1f - 0.5f);
					camera_node->set_pos(tar + camera_node->g_rot[2] * camera_zoom);
				}

				if (all(greaterThanEqual((vec2)io.MousePos, (vec2)p0)) && all(lessThanEqual((vec2)io.MousePos, (vec2)p1)))
				{
					auto hovering_node = sNodeRenderer::instance()->pick_up((vec2)io.MousePos - (vec2)p0);
				}
			}
		}
	}
}
