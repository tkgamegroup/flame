#include "view_scene.h"
#include "selection.h"

#include <flame/universe/components/node.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/camera.h>

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

#if USE_IM_GUIZMO
		if (app.e_editor && selection.type == Selection::tEntity)
		{
			auto tar = selection.entity->get_component_i<cNode>(0);
			if (tar)
			{
				auto camera = app.e_editor->get_component_t<cCamera>();

				ImGuizmo::BeginFrame();
				ImGuizmo::SetRect(p0.x, p0.y, p1.x - p0.x, p1.y - p0.y);
				auto matp = camera->proj_mat; matp[1][1] *= -1.f;
				auto mat = tar->transform;
				ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
				if (ImGuizmo::Manipulate(&camera->view_mat[0][0], &matp[0][0], ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, &mat[0][0]))
				{
					vec3 p, r, s;
					ImGuizmo::DecomposeMatrixToComponents(&mat[0][0], &p[0], &r[0], &s[0]);
					tar->set_pos(p);
					//tar->set_eul(r);
					tar->set_scl(s);
				}
			}
		}
#endif

		if (ImGui::IsItemHovered())
		{
			if (app.e_editor)
			{
				auto editor_node = app.e_editor->get_component_i<cNode>(0);
				if (!editor_node->drawers.exist("scene"_h))
				{
					editor_node->drawers.add([this](sNodeRendererPtr renderer, bool shadow_pass) {
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
					}, "scene"_h);
					editor_node->measurers.add([](AABB* ret) {
						*ret = AABB(vec3(0.f), 10000.f);
						return true;
					});
					editor_node->mark_transform_dirty();
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
						if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !io.KeyAlt && !io.KeyShift)
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
