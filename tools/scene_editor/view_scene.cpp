#include "view_scene.h"
#include "selection.h"

#include <flame/foundation/typeinfo.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/armature.h>
#include <flame/universe/components/terrain.h>

View_Scene view_scene;

View_Scene::View_Scene() :
	View("Scene")
{
}

cCameraPtr View_Scene::curr_camera()
{
	return cCamera::list()[camera_idx];
}

void View_Scene::focus_to_selected()
{
	if (selection.type == Selection::tEntity)
	{
		if (auto node = selection.entity()->get_component_i<cNodeT>(0); node)
		{
			auto camera_node = curr_camera()->node;
			camera_node->set_pos(node->g_pos + camera_node->g_rot[2] * camera_zoom);
		}
	}
}

void View_Scene::selected_to_focus()
{
	if (selection.type == Selection::tEntity)
	{
		if (auto node = selection.entity()->get_component_i<cNode>(0); node)
		{
			auto camera_node = curr_camera()->node;
			node->set_pos(camera_node->g_pos - camera_node->g_rot[2] * camera_zoom);
		}
	}
}

void View_Scene::on_draw()
{
	auto& camera_list = cCamera::list();
	{
		static const char* names[8];
		auto n = min(countof(names), camera_list.size());
		for (auto i = 0; i < n; i++)
			names[i] = camera_list[i]->entity->name.c_str();
		ImGui::Combo("Camera", (int*)&camera_idx, names, n);
	}
	auto camera = camera_list[camera_idx];
	app.renderer->camera = camera;

	auto scene_size = vec2(ImGui::GetContentRegionAvail());
	if (fixed_render_target_size)
		scene_size = vec2(1280, 720);
	if (!render_tar || vec2(render_tar->size) != scene_size)
	{
		add_event([this, scene_size]() {
			graphics::Queue::get()->wait_idle();
			if (scene_size.x > 1 && scene_size.y > 1)
			{
				render_tar.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec2(scene_size),
					graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
				render_tar->change_layout(graphics::ImageLayoutShaderReadOnly);
				auto iv = render_tar->get_view();
				app.renderer->set_targets({ &iv, 1 }, graphics::ImageLayoutShaderReadOnly);
			}
			else
			{
				render_tar.reset();
				app.renderer->set_targets({}, graphics::ImageLayoutShaderReadOnly);
			}
			return false;
		});
	}

	hovering_node = nullptr;

	if (render_tar)
	{
		ImGui::Image(render_tar.get(), scene_size);
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		app.input->offset = p0;

		bool using_gizmo = false;
#if USE_IM_GUIZMO
		if (is_in(app.tool, ToolMove, ToolScale) && app.e_editor && selection.type == Selection::tEntity)
		{
			auto e = selection.entity();
			auto tar = e->get_component_i<cNode>(0);
			if (tar)
			{

				ImGuizmo::BeginFrame();
				ImGuizmo::SetRect(p0.x, p0.y, p1.x - p0.x, p1.y - p0.y);
				auto matp = camera->proj_mat; matp[1][1] *= -1.f;
				auto mat = tar->transform;
				ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
				auto op = ImGuizmo::TRANSLATE;
				switch (app.tool)
				{
				case ToolMove: op = ImGuizmo::TRANSLATE; break;
				case ToolRotate: op = ImGuizmo::ROTATE; break;
				case ToolScale: op = ImGuizmo::SCALE; break;
				}
				if (ImGuizmo::Manipulate(&camera->view_mat[0][0], &matp[0][0], op, ImGuizmo::LOCAL, &mat[0][0]))
				{
					vec3 p, r, s;
					ImGuizmo::DecomposeMatrixToComponents(&mat[0][0], &p[0], &r[0], &s[0]);
					r = vec3(r.y, r.x, r.z);
					if (p != tar->pos)
					{
						tar->set_pos(p);
						if (auto ins = get_prefab_instance(e); ins)
							ins->mark_modifier(e->file_id, "flame::cNode", "pos");
					}
					if (r != tar->eul)
					{
						tar->set_eul(r);
						if (auto ins = get_prefab_instance(e); ins)
							ins->mark_modifier(e->file_id, "flame::cNode", "eul");
					}
					if (s != tar->scl)
					{
						tar->set_scl(s);
						if (auto ins = get_prefab_instance(e); ins)
							ins->mark_modifier(e->file_id, "flame::cNode", "scl");
					}
				}

				using_gizmo = ImGuizmo::IsUsing();
			}
		}
#endif

		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		auto editor_node = app.e_editor->get_component_i<cNode>(0);
		if (!editor_node->drawers.exist("scene"_h))
		{
			editor_node->drawers.add([this](sRendererPtr renderer) {
				auto outline_node = [&](EntityPtr e, const cvec4& col) {
					if (auto mesh = e->get_component_t<cMesh>(); mesh && mesh->instance_id != -1 && mesh->mesh_res_id != -1)
						renderer->draw_mesh_outline(mesh->instance_id, mesh->mesh_res_id, col);
					if (auto terrain = e->get_component_t<cTerrain>(); terrain && terrain->instance_id != -1 && terrain->height_map)
						renderer->draw_terrain_outline(terrain->instance_id, terrain->blocks.x * terrain->blocks.y, col);
				};
				if (hovering_node && selection.selecting(hovering_node->entity))
					outline_node(hovering_node->entity, cvec4(178, 178, 96, 255));
				else
				{
					if (hovering_node)
						outline_node(hovering_node->entity, cvec4(128, 128, 64, 255));
					if (selection.type == Selection::tEntity)
						outline_node(selection.entity(), cvec4(255, 255, 128, 255));
				}
				if (show_AABB)
				{
					World::instance()->root->forward_traversal([renderer](EntityPtr e) {
						if (!e->global_enable)
							return false;
						if (auto node = e->get_component_i<cNode>(0); node)
						{
							if (!node->bounds.invalid())
							{
								auto points = node->bounds.get_points();
								vec3 line_pts[24];
								auto p = line_pts;
								*p++ = points[0]; *p++ = points[1];
								*p++ = points[1]; *p++ = points[2];
								*p++ = points[2]; *p++ = points[3];
								*p++ = points[3]; *p++ = points[0];
								*p++ = points[0]; *p++ = points[4];
								*p++ = points[1]; *p++ = points[5];
								*p++ = points[2]; *p++ = points[6];
								*p++ = points[3]; *p++ = points[7];
								*p++ = points[4]; *p++ = points[5];
								*p++ = points[5]; *p++ = points[6];
								*p++ = points[6]; *p++ = points[7];
								*p++ = points[7]; *p++ = points[4];
								renderer->draw_line(line_pts, countof(line_pts), cvec4(255, 127, 127, 255));
							}
						}
						return true;
					});
				}
				if (show_axis)
				{
					if (selection.type == Selection::tEntity)
					{
						auto e = selection.entity();
						if (e->global_enable)
						{
							if (auto node = e->get_component_i<cNode>(0); node)
							{
								vec3 line_pts[2];
								line_pts[0] = node->g_pos; line_pts[1] = node->g_pos + node->g_rot[0];
								renderer->draw_line(line_pts, countof(line_pts), cvec4(255, 0, 0, 255));
								line_pts[0] = node->g_pos; line_pts[1] = node->g_pos + node->g_rot[1];
								renderer->draw_line(line_pts, countof(line_pts), cvec4(0, 255, 0, 255));
								line_pts[0] = node->g_pos; line_pts[1] = node->g_pos + node->g_rot[2];
								renderer->draw_line(line_pts, countof(line_pts), cvec4(0, 0, 255, 255));
							}
						}
					}
				}
				if (show_bones)
				{
					World::instance()->root->forward_traversal([renderer](EntityPtr e) {
						if (!e->global_enable)
							return false;
						if (auto arm = e->get_component_t<cArmature>(); arm)
						{
							std::function<void(cNodePtr)> draw_node;
							draw_node = [&, renderer](cNodePtr n) {
								vec3 line_pts[2];
								line_pts[0] = n->g_pos;
								for (auto& c : n->entity->children)
								{
									auto nn = c->get_component_t<cNode>();
									if (nn)
									{
										line_pts[1] = nn->g_pos;
										renderer->draw_line(line_pts, countof(line_pts), cvec4(255));
										draw_node(nn);
									}
								}
							};
							draw_node(arm->node);
						}
						return true;
					});
				}
				}, "scene"_h);
			editor_node->mark_transform_dirty();
		}

		if (ImGui::IsItemHovered())
		{
			auto camera_node = camera->node;

			auto get_tar = [&]() {
				return camera_node->g_pos - camera_node->g_rot[2] * camera_zoom;
			};

			if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
			{
				disp /= vec2(render_tar->size);
				if (!io.KeyAlt)
				{
					if (io.MouseDown[ImGuiMouseButton_Middle])
					{
						camera_node->add_pos((-camera_node->g_rot[0] * disp.x +
							camera_node->g_rot[1] * disp.y) * camera_zoom);
					}
					else if (io.MouseDown[ImGuiMouseButton_Right])
					{
						disp *= -180.f;
						camera_node->add_eul(vec3(disp, 0.f));
					}
				}
				else
				{
					if (io.MouseDown[ImGuiMouseButton_Left])
					{
						disp *= -180.f;
						auto tar = get_tar();
						camera_node->add_eul(vec3(disp, 0.f));
						auto eul = camera_node->eul;
						auto rot = mat3(eulerAngleYXZ(radians(eul.x), radians(eul.y), radians(eul.z)));
						camera_node->set_pos(tar + rot[2] * camera_zoom);
					}
				}
			}
			{
				static vec2 last_mpos = vec2(0.f);
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
					last_mpos = io.MousePos;
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle) && (vec2)io.MousePos == last_mpos)
					camera_node->set_pos(hovering_pos + camera_node->g_rot[2] * camera_zoom);
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
			if (!io.WantCaptureKeyboard)
			{
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
					focus_to_selected();
				if (io.KeysDown[Keyboard_G])
					selected_to_focus();
				if (ImGui::IsKeyPressed(Keyboard_Del))
					app.cmd_delete_entity();
			}

			if (all(greaterThanEqual((vec2)io.MousePos, (vec2)p0)) && all(lessThanEqual((vec2)io.MousePos, (vec2)p1)))
			{
				hovering_node = sRenderer::instance()->pick_up((vec2)io.MousePos - (vec2)p0, &hovering_pos);
				if (!using_gizmo && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyAlt)
				{
					if (hovering_node)
						selection.select(hovering_node->entity);
					else
						selection.clear();
				}
				{
					auto s = str(hovering_pos);
					auto sz = ImGui::CalcTextSize(s.c_str(), s.c_str() + s.size());
					ImGui::GetWindowDrawList()->AddRectFilled(p0, (vec2)p0 + (vec2)sz, ImColor(0.f, 0.f, 0.f, 0.5f));
					ImGui::GetWindowDrawList()->AddText(p0, ImColor(255.f, 255.f, 255.f), s.c_str(), s.c_str() + s.size());
				}
			}
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
			{
				if (app.e_prefab)
				{
					auto str = std::wstring((wchar_t*)payload->Data);
					auto path = Path::reverse(str);
					if (path.extension() == L".prefab")
					{
						auto e = Entity::create();
						e->load(path);
						new PrefabInstance(e, path);
						if (auto node = e->get_component_i<cNode>(0); node)
							node->set_pos(hovering_pos);
						app.e_prefab->add_child(e);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
}
