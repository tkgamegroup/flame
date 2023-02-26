#include "view_scene.h"
#include "selection.h"

#include <flame/foundation/typeinfo.h>
#include <flame/universe/draw_data.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/armature.h>
#include <flame/universe/components/terrain.h>
#include <flame/universe/components/volume.h>
#include <flame/universe/components/nav_agent.h>
#include <flame/universe/components/nav_obstacle.h>

View_Scene view_scene;

View_Scene::View_Scene() :
	GuiView("Scene")
{
}

cCameraPtr View_Scene::curr_camera()
{
	return cCamera::list()[camera_idx];
}

vec3 View_Scene::camera_target_pos()
{
	auto camera_node = curr_camera()->node;
	return camera_node->g_pos - camera_node->g_rot[2] * camera_zoom;
}

void View_Scene::focus_to_selected()
{
	if (selection.type == Selection::tEntity)
	{
		if (auto node = selection.entity()->node(); node)
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
		auto e = selection.entity();
		if (auto node = e->get_component_i<cNode>(0); node)
		{
			node->set_pos(camera_target_pos());
			if (auto ins = get_prefab_instance(e); ins)
				ins->mark_modifier(e->file_id, "flame::cNode", "pos");
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

	auto scene_extent = vec2(ImGui::GetContentRegionAvail());
	if (fixed_render_target_size)
		scene_extent = vec2(1280, 720);
	if (!render_tar || vec2(render_tar->extent) != scene_extent)
	{
		add_event([this, scene_extent]() {
			graphics::Queue::get()->wait_idle();
			if (scene_extent.x > 1 && scene_extent.y > 1)
			{
				render_tar.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(scene_extent, 1),
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
		ImGui::Image(render_tar.get(), scene_extent);
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		app.input->offset = p0;

		bool gizmo_using = false;
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
				vec3 snap_value; float* p_snap_value = nullptr;
				switch (app.tool)
				{
				case ToolMove: 
					op = ImGuizmo::TRANSLATE;
					if (app.move_snap)
					{
						snap_value = vec3(app.move_snap_value);
						p_snap_value = &snap_value[0];
					}
					break;
				case ToolRotate: 
					op = ImGuizmo::ROTATE;
					if (app.rotate_snap)
					{
						snap_value = vec3(app.rotate_snap_value);
						p_snap_value = &snap_value[0];
					}
					break;
				case ToolScale: 
					op = ImGuizmo::SCALE;
					if (app.scale_snap)
					{
						snap_value = vec3(app.scale_snap_value);
						p_snap_value = &snap_value[0];
					}
					break;
				}
				auto changed = ImGuizmo::Manipulate(&camera->view_mat[0][0], &matp[0][0], op, app.tool_mode == ToolLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD, &mat[0][0], nullptr, p_snap_value);
				static bool last_gizmo_using = false;
				static vec3 before_editing_pos;
				static quat before_editing_qut;
				static vec3 before_editing_scl;
				gizmo_using = ImGuizmo::IsUsing();
				if (!last_gizmo_using && gizmo_using)
					before_editing_pos = tar->pos;
				if (changed)
				{
					if (auto pnode = e->get_parent_component_i<cNodeT>(0); pnode)
						mat = inverse(pnode->transform) * mat;
					vec3 pos; quat qut; vec3 scl; vec3 skew; vec4 perspective;
					decompose(mat, scl, qut, pos, skew, perspective);
					if (app.tool == ToolMove && pos != tar->pos)
					{
						tar->set_pos(pos);
						if (auto ins = get_prefab_instance(e); ins)
							ins->mark_modifier(e->file_id, "flame::cNode", "pos");
					}
					if (app.tool == ToolRotate && qut != tar->qut)
					{
						tar->set_qut(qut);
						if (auto ins = get_prefab_instance(e); ins)
							ins->mark_modifier(e->file_id, "flame::cNode", "qut");
					}
					if (app.tool == ToolScale && scl != tar->scl)
					{
						tar->set_scl(scl);
						if (auto ins = get_prefab_instance(e); ins)
							ins->mark_modifier(e->file_id, "flame::cNode", "scl");
					}
				}
				if (last_gizmo_using && !gizmo_using)
				{
					if (app.tool == ToolMove && before_editing_pos != tar->pos)
						add_history(new EntityModifyHistory(e->instance_id, "flame::cNode"_h, "pos"_h, str(before_editing_pos), str(tar->pos)));
					if (app.tool == ToolRotate && before_editing_qut != tar->qut)
						add_history(new EntityModifyHistory(e->instance_id, "flame::cNode"_h, "qut"_h, str(*(vec4*)&before_editing_qut), str(*(vec4*)&tar->qut)));
					if (app.tool == ToolScale && before_editing_scl != tar->scl)
						add_history(new EntityModifyHistory(e->instance_id, "flame::cNode"_h, "scl"_h, str(before_editing_scl), str(tar->scl)));
				}
				last_gizmo_using = gizmo_using;
			}
		}
#endif

		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		auto editor_node = app.e_editor->get_component_i<cNode>(0);
		if (editor_node->drawers.find("scene"_h) == -1)
		{
			editor_node->drawers.add([this](DrawData& draw_data) {
				if (draw_data.pass == PassOutline && show_outline)
				{
					auto outline_node = [&](EntityPtr e, const cvec4& col) {
						if (auto mesh = e->get_component_t<cMesh>(); mesh && mesh->instance_id != -1 && mesh->mesh_res_id != -1)
							draw_data.meshes.emplace_back(mesh->instance_id, mesh->mesh_res_id, 0, col);
						if (auto terrain = e->get_component_t<cTerrain>(); terrain && terrain->instance_id != -1 && terrain->height_map)
							draw_data.terrains.emplace_back(terrain->instance_id, terrain->blocks, 0, col);
						if (auto armature = e->get_component_t<cArmature>(); armature && armature->model)
						{
							auto idx = (int)draw_data.meshes.size();
							for (auto& c : e->children)
							{
								if (auto mesh = c->get_component_t<cMesh>(); mesh && mesh->instance_id != -1 && mesh->mesh_res_id != -1)
									draw_data.meshes.emplace_back(mesh->instance_id, mesh->mesh_res_id, -1, col);
							}
							if (draw_data.meshes.size() > idx)
								draw_data.meshes.back().mat_id = 0;
						}
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
				}
				if (draw_data.pass == PassPrimitive)
				{
					if (show_AABB)
					{
						World::instance()->root->forward_traversal([&draw_data](EntityPtr e) {
							if (!e->global_enable)
								return false;
							if (auto node = e->get_component_i<cNode>(0); node)
							{
								if (!node->bounds.invalid())
								{
									auto points = node->bounds.get_points();
									auto line_pts = Frustum::points_to_lines(points.data());
									draw_data.primitives.emplace_back("LineList"_h, std::move(line_pts), cvec4(255, 127, 127, 255));
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
									draw_data.primitives.emplace_back("LineList"_h, line_pts, 2, cvec4(255, 0, 0, 255));
									line_pts[0] = node->g_pos; line_pts[1] = node->g_pos + node->g_rot[1];
									draw_data.primitives.emplace_back("LineList"_h, line_pts, 2, cvec4(0, 255, 0, 255));
									line_pts[0] = node->g_pos; line_pts[1] = node->g_pos + node->g_rot[2];
									draw_data.primitives.emplace_back("LineList"_h, line_pts, 2, cvec4(0, 0, 255, 255));
								}
							}
						}
					}
					if (show_bones)
					{
						World::instance()->root->forward_traversal([&draw_data](EntityPtr e) {
							if (!e->global_enable)
								return false;
							if (auto arm = e->get_component_t<cArmature>(); arm)
							{
								std::function<void(cNodePtr)> draw_node;
								draw_node = [&](cNodePtr n) {
									vec3 line_pts[2];
									line_pts[0] = n->g_pos;
									for (auto& c : n->entity->children)
									{
										auto nn = c->node();
										if (nn)
										{
											line_pts[1] = nn->g_pos;
											draw_data.primitives.emplace_back("LineList"_h, line_pts, 2, cvec4(255));
											draw_node(nn);
										}
									}
								};
								draw_node(arm->node);
							}
							return true;
						});
					}
					if (show_navigation)
					{
						World::instance()->root->forward_traversal([&draw_data](EntityPtr e) {
							if (!e->global_enable)
								return false;
							auto draw_cylinder = [&](const vec3& p, float r, float h) {
								auto circle_pts = graphics::get_circle_points(r > 8.f ? 3 : (r > 4.f ? 3 : (r > 2.f ? 2 : (r > 1.f ? 1 : 0))));
								auto n = (int)circle_pts.size();
								circle_pts.push_back(circle_pts[0]);
								std::vector<vec3> pts(n * 2);
								auto center = p;
								for (auto i = 0; i < n; i++)
								{
									pts[i * 2 + 0] = center + vec3(r * circle_pts[i + 0], 0.f).xzy();
									pts[i * 2 + 1] = center + vec3(r * circle_pts[i + 1], 0.f).xzy();
								}
								draw_data.primitives.emplace_back("LineList"_h, pts.data(), (uint)pts.size(), cvec4(127, 0, 255, 255));
								center.y += h;
								for (auto i = 0; i < n; i++)
								{
									pts[i * 2 + 0] = center + vec3(r * circle_pts[i + 0], 0.f).xzy();
									pts[i * 2 + 1] = center + vec3(r * circle_pts[i + 1], 0.f).xzy();
								}
								draw_data.primitives.emplace_back("LineList"_h, pts.data(), (uint)pts.size(), cvec4(127, 0, 255, 255));
								center = p;
								pts[0] = center + r * vec3(+1.f, 0.f, 0.f);
								pts[1] = pts[0] + vec3(0.f, h, 0.f);
								draw_data.primitives.emplace_back("LineList"_h, pts.data(), 2, cvec4(127, 0, 255, 255));
								pts[0] = center + r * vec3(-1.f, 0.f, 0.f);
								pts[1] = pts[0] + vec3(0.f, h, 0.f);
								draw_data.primitives.emplace_back("LineList"_h, pts.data(), 2, cvec4(127, 0, 255, 255));
								pts[0] = center + r * vec3(0.f, 0.f, +1.f);
								pts[1] = pts[0] + vec3(0.f, h, 0.f);
								draw_data.primitives.emplace_back("LineList"_h, pts.data(), 2, cvec4(127, 0, 255, 255));
								pts[0] = center + r * vec3(0.f, 0.f, -1.f);
								pts[1] = pts[0] + vec3(0.f, h, 0.f);
								draw_data.primitives.emplace_back("LineList"_h, pts.data(), 2, cvec4(127, 0, 255, 255));
							};
							if (auto agent = e->get_component_t<cNavAgent>(); agent)
								draw_cylinder(agent->node->g_pos, agent->radius, agent->height);
							if (auto obstacle = e->get_component_t<cNavObstacle>(); obstacle)
								draw_cylinder(obstacle->node->g_pos, obstacle->radius, obstacle->height);
							return true;
						});

						sScene::instance()->get_debug_draw(draw_data);
					}
				}
			}, "scene"_h);
			editor_node->mark_transform_dirty();
		}

		if (!app.e_playing || app.control)
		{
			if (ImGui::IsItemHovered())
			{
				auto camera_node = camera->node;

				auto get_tar = [&]() {
					return camera_node->g_pos - camera_node->g_rot[2] * camera_zoom;
				};

				if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
				{
					disp /= vec2(render_tar->extent);
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
					if (!io.KeysDown[Keyboard_Ctrl] && !io.KeysDown[Keyboard_Alt] && !io.KeysDown[Keyboard_Shift])
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
						if (ImGui::IsKeyPressed(Keyboard_1))
							app.tool = ToolSelect;
						if (ImGui::IsKeyPressed(Keyboard_2))
							app.tool = ToolMove;
						if (ImGui::IsKeyPressed(Keyboard_3))
							app.tool = ToolRotate;
						if (ImGui::IsKeyPressed(Keyboard_4))
							app.tool = ToolScale;
					}
				}

				if (all(greaterThanEqual((vec2)io.MousePos, (vec2)p0)) && all(lessThanEqual((vec2)io.MousePos, (vec2)p1)))
				{
					hovering_node = sRenderer::instance()->pick_up((vec2)io.MousePos - (vec2)p0, &hovering_pos, [](cNodePtr n, DrawData& draw_data) {
						if (draw_data.categories & CateMesh)
						{
							if (auto armature = n->entity->get_component_t<cArmature>(); armature)
							{
								for (auto& c : n->entity->children)
								{
									if (auto mesh = c->get_component_t<cMesh>(); mesh)
									{
										if (mesh->instance_id != -1 && mesh->mesh_res_id != -1 && mesh->material_res_id != -1)
											draw_data.meshes.emplace_back(mesh->instance_id, mesh->mesh_res_id, mesh->material_res_id);
									}
								}
							}
							if (auto mesh = n->entity->get_component_t<cMesh>(); mesh)
							{
								if (auto armature = n->entity->get_parent_component_t<cArmature>(); armature)
									;
								else
								{
									if (mesh->instance_id != -1 && mesh->mesh_res_id != -1 && mesh->material_res_id != -1)
										draw_data.meshes.emplace_back(mesh->instance_id, mesh->mesh_res_id, mesh->material_res_id);
								}
							}
						}
						if (draw_data.categories & CateTerrain)
						{
							if (auto terrain = n->entity->get_component_t<cTerrain>(); terrain)
								draw_data.terrains.emplace_back(terrain->instance_id, terrain->blocks, terrain->material_res_id);
						}
						if (draw_data.categories & CateMarchingCubes)
						{
							if (auto volume = n->entity->get_component_t<cVolume>(); volume && volume->marching_cubes)
								draw_data.volumes.emplace_back(volume->instance_id, volume->blocks, volume->material_res_id);
						}
					});
					if (!gizmo_using && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyAlt)
					{
						if (hovering_node)
							selection.select(hovering_node->entity, "scene"_h);
						else
							selection.clear("scene"_h);
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
						auto ext = path.extension();
						if (ext == L".prefab")
						{
							add_event([this, path]() {
								auto e = Entity::create();
								e->load(path);
								new PrefabInstance(e, path);
								if (auto node = e->get_component_i<cNode>(0); node)
								{
									auto pos = hovering_pos;
									if (!hovering_node)
									{
										auto camera_node = view_scene.curr_camera()->node;
										auto camera_pos = camera_node->g_pos;
										auto v = normalize(pos - camera_pos);
										pos = camera_pos + v * (view_scene.camera_zoom / dot(v, -camera_node->g_rot[2]));
									}
									node->set_pos(app.get_snap_pos(pos));
								}
								if (app.e_playing)
									app.e_playing->add_child(e);
								else
									app.e_prefab->add_child(e);
								return false;
							});
						}
						else if (ext == L".fmat")
						{
							if (hovering_node)
							{
								if (auto mesh = hovering_node->entity->get_component_t<cMesh>(); mesh)
									mesh->set_material_name(path);
							}
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
	}
}

bool View_Scene::on_begin()
{
	bool open = true;
	ImGui::Begin(name.c_str(), &open, app.control ? 0 : (ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoDocking));
	return !open;
}
