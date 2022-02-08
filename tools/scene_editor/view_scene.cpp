#include <flame/universe/components/node.h>
#include <flame/universe/components/mesh.h>
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
		render_tar.reset(graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, uvec2(size), 
			graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
		render_tar->change_layout(graphics::ImageLayoutShaderReadOnly);
		auto iv = render_tar->get_view();
		app.node_renderer->set_targets( { &iv, 1 }, graphics::ImageLayoutShaderReadOnly );
	}

	hovering_node = nullptr;

	if (render_tar)
	{
		ImGui::Image(render_tar.get(), size);
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		if (ImGui::IsItemHovered())
		{
			if (app.e_prefab)
			{
				if (!editor_node)
				{
					if (auto e = app.e_prefab->find_child("[Editor]"); e)
					{
						editor_node = e->get_component_i<cNodeT>(0);
						editor_drawer = editor_node->drawers.add([this](sNodeRendererPtr renderer, bool shadow_pass) {
							auto outline_node = [&](EntityPtr e, const cvec4& col) {
								auto mesh = e->get_component_t<cMesh>();
								if (mesh && mesh->object_id != -1 && mesh->mesh_id != -1)
									renderer->draw_mesh_outline(mesh->object_id, mesh->mesh_id, col);
							};
							if (hovering_node && selection.selecting(hovering_node->entity))
								outline_node(hovering_node->entity, cvec4(178, 178, 96, 0));
							else
							{
								if (hovering_node)
									outline_node(hovering_node->entity, cvec4(128, 128, 64, 0));
								if (selection.type == Selection::tEntity)
									outline_node(selection.entity, cvec4(255, 255, 128, 0));
							}
						});
						editor_node->measurers.add([](AABB* ret) {
							*ret = AABB(vec3(0.f), 10000.f);
							return true;
						});
						editor_node->mark_transform_dirty();
					}
				}
				if (editor_node)
				{
					auto get_tar = [&]() {
						return editor_node->g_pos - editor_node->g_rot[2] * camera_zoom;
					};

					auto& io = ImGui::GetIO();
					if (io.KeyAlt)
					{
						if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
						{
							disp /= vec2(render_tar->size);
							disp *= -180.f;
							if (io.MouseDown[ImGuiMouseButton_Left])
								editor_node->add_eul(vec3(disp, 0.f));
							else if (io.MouseDown[ImGuiMouseButton_Middle])
							{
								auto tar = get_tar();
								editor_node->add_eul(vec3(disp, 0.f));
								auto eul = editor_node->eul;
								auto rot = mat3(eulerAngleYXZ(radians(eul.x), radians(eul.y), radians(eul.z)));
								editor_node->set_pos(tar + rot[2] * camera_zoom);
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
								editor_node->add_pos((-editor_node->g_rot[0] * disp.x +
									editor_node->g_rot[1] * disp.y) * camera_zoom);
							}
						}
					}
					if (io.KeysDown[Keyboard_W])
					{
						editor_node->add_pos(-editor_node->g_rot[2] * 0.2f);
						app.render_frames += 30;
					}
					if (io.KeysDown[Keyboard_S])
					{
						editor_node->add_pos(+editor_node->g_rot[2] * 0.2f);
						app.render_frames += 30;
					}
					if (io.KeysDown[Keyboard_A])
					{
						editor_node->add_pos(-editor_node->g_rot[0] * 0.2f);
						app.render_frames += 30;
					}
					if (io.KeysDown[Keyboard_D])
					{
						editor_node->add_pos(+editor_node->g_rot[0] * 0.2f);
						app.render_frames += 30;
					}
					if (io.KeysDown[Keyboard_F])
					{
						if (selection.type == Selection::tEntity)
						{
							if (auto node = selection.entity->get_component_i<cNodeT>(0); node)
								editor_node->set_pos(node->g_pos + editor_node->g_rot[2] * camera_zoom);
						}
					}
					if (auto scroll = io.MouseWheel; scroll != 0.f)
					{
						auto tar = get_tar();
						if (scroll < 0.f)
							camera_zoom = camera_zoom * 1.1f + 0.5f;
						else
							camera_zoom = max(0.f, camera_zoom / 1.1f - 0.5f);
						editor_node->set_pos(tar + editor_node->g_rot[2] * camera_zoom);
					}

					if (all(greaterThanEqual((vec2)io.MousePos, (vec2)p0)) && all(lessThanEqual((vec2)io.MousePos, (vec2)p1)))
					{
						hovering_node = sNodeRenderer::instance()->pick_up((vec2)io.MousePos - (vec2)p0);
						if (io.MouseDown[0])
						{
							if (hovering_node)
								selection.select(hovering_node->entity);
							else
								selection.clear();
						}
					}
				}
			}
		}
	}
}
