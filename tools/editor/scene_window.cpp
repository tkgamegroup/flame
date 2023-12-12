#include "scene_window.h"
#include "selection.h"
#include "history.h"
#include "tile_map_editing.h"

#include <flame/universe/draw_data.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/armature.h>
#include <flame/universe/components/terrain.h>
#include <flame/universe/components/volume.h>
#include <flame/universe/components/nav_agent.h>
#include <flame/universe/components/nav_obstacle.h>

SceneWindow scene_window;

vec3 SceneView::get_snap_pos(const vec3& _pos)
{
	auto pos = _pos;
	if (move_snap)
	{
		pos /= move_snap_value;
		pos -= fract(pos);
		pos *= move_snap_value;
	}
	return pos;
}

SceneView::SceneView() :
	SceneView(scene_window.views.empty() ? "Scene" : "Scene##" + str(rand()))
{
}

SceneView::SceneView(const std::string& name) :
	View(&scene_window, name)
{
}

cCameraPtr SceneView::curr_camera()
{
	return cCamera::list()[camera_idx];
}

vec3 SceneView::camera_target_pos()
{
	auto camera_node = curr_camera()->node;
	return camera_node->global_pos() - camera_node->z_axis() * camera_zoom;
}

void SceneView::reset_camera(uint op)
{
	auto camera_node = curr_camera()->node;

	switch (op)
	{
	case ""_h:
		camera_zoom = 5.f;
		camera_node->set_qut(quat(1.f, 0.f, 0.f, 0.f));
		camera_node->set_pos(vec3(0.f, 0.f, 5.f));
		break;
	case "X+"_h:
		camera_node->set_qut(angleAxis(radians(+90.f), vec3(0.f, 1.f, 0.f)));
		camera_node->set_pos(vec3(+camera_zoom, 0.f, 0.f));
		break;
	case "X-"_h:
		camera_node->set_qut(angleAxis(radians(-90.f), vec3(0.f, 1.f, 0.f)));
		camera_node->set_pos(vec3(-camera_zoom, 0.f, 0.f));
		break;
	case "Y+"_h:
		camera_node->set_qut(angleAxis(radians(-90.f), vec3(1.f, 0.f, 0.f)));
		camera_node->set_pos(vec3(0.f, +camera_zoom, 0.f));
		break;
	case "Y-"_h:
		camera_node->set_qut(angleAxis(radians(+90.f), vec3(1.f, 0.f, 0.f)));
		camera_node->set_pos(vec3(0.f, -camera_zoom, 0.f));
		break;
	case "Z+"_h:
		camera_node->set_qut(quat(1.f, 0.f, 0.f, 0.f));
		camera_node->set_pos(vec3(0.f, 0.f, +camera_zoom));
		break;
	case "Z-"_h:
		camera_node->set_qut(angleAxis(radians(180.f), vec3(0.f, 1.f, 0.f)));
		camera_node->set_pos(vec3(0.f, 0.f, -camera_zoom));
		break;
	}
}

void SceneView::focus_to_selected()
{
	if (selection.type == Selection::tEntity)
	{
		if (auto node = selection.as_entity()->get_component<cNode>(); node)
		{
			AABB bounds;
			node->entity->forward_traversal([&](EntityPtr e) {
				if (auto node = e->get_component<cNode>(); node)
				{
					if (!node->bounds.invalid())
						bounds.expand(node->bounds);
				}
			});

			auto camera = curr_camera();
			auto camera_node = camera->node;
			if (!bounds.invalid())
			{
				auto pos = fit_camera_to_object(mat3(camera_node->g_qut), camera->fovy, camera->zNear, camera->aspect, bounds);
				camera_zoom = distance(pos, bounds.center());
				camera_node->set_pos(pos);
			}
			else
				camera_node->set_pos(node->global_pos() + camera_node->z_axis() * camera_zoom);
		}
	}
}

void SceneView::selected_to_focus()
{
	if (selection.type == Selection::tEntity)
	{
		auto e = selection.as_entity();
		if (auto node = e->get_component<cNode>(); node)
		{
			node->set_pos(camera_target_pos());
			if (auto ins = get_root_prefab_instance(e); ins)
				ins->mark_modification(e->file_id.to_string() + "|flame::cNode|pos");
		}
	}
}

void SceneView::on_draw()
{
	std::string open_name;
	if (app.e_prefab)
	{
		open_name = "Scene - " + app.prefab_path.filename().stem().string();
		open_name += "###";
		open_name += name;
	}
	else
		open_name = "###" + name;

	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(open_name.c_str(), &opened, app.prefab_unsaved ? ImGuiWindowFlags_UnsavedDocument : 0);
	imgui_window = ImGui::GetCurrentWindow();
	// there is a bug that ImGui do not reset the pointer to window's name in draw list, so that
	//  ImGuizmo not work properly
	ImGui::GetWindowDrawList()->_OwnerName = ImGui::GetCurrentWindow()->Name;

	auto& camera_list = cCamera::list();
	{
		static const char* names[8];
		auto n = min(countof(names), camera_list.size());
		for (auto i = 0; i < n; i++)
			names[i] = camera_list[i]->entity->name.c_str();
		ImGui::SetNextItemWidth(100.f);
		ImGui::Combo("Camera", (int*)&camera_idx, names, n);
	}
	auto camera = camera_list[camera_idx];
	app.renderer->render_tasks.front()->camera = camera;
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		auto camera_node = camera->node;
		camera_node->set_pos(vec3(0.f));
		camera_node->set_qut(quat(1.f, 0.f, 0.f, 0.f));
	}

	ImGui::SameLine();
	if (ImGui::ToolButton("Outline", show_outline))
		show_outline = !show_outline;
	ImGui::SameLine();
	if (ImGui::ToolButton("AABB", show_AABB))
		show_AABB = !show_AABB;
	if (show_AABB)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.f);
		if (ImGui::BeginCombo("##AABB_combo", !show_AABB_only_selected ? "All" : "Only Selected"))
		{
			if (ImGui::Selectable("All", !show_AABB_only_selected))
				show_AABB_only_selected = false;
			if (ImGui::Selectable("Only Selected", show_AABB_only_selected))
				show_AABB_only_selected = true;
			ImGui::EndCombo();
		}
	}
	ImGui::SameLine();
	if (ImGui::ToolButton("Axis", show_axis))
		show_axis = !show_axis;
	ImGui::SameLine();
	if (ImGui::ToolButton("Bones", show_bones))
		show_bones = !show_bones;
	ImGui::SameLine();
	if (ImGui::ToolButton("Navigation", show_navigation))
		show_navigation = !show_navigation;
	ImGui::SameLine();
	if (ImGui::ToolButton("Gizmos", use_gizmos))
		use_gizmos = !use_gizmos;

	auto last_focused_scene = scene_window.first_view();

	// toolbar begin
	ImGui::Dummy(vec2(0.f, 20.f));
	ImGui::SameLine();
	if (ImGui::ToolButton(graphics::font_icon_str("arrow-pointer"_h).c_str(), tool == ToolSelect))
		tool = ToolSelect;
	ImGui::SameLine();
	if (ImGui::ToolButton(graphics::font_icon_str("arrows-up-down-left-right"_h).c_str(), tool == ToolMove))
		tool = ToolMove;
	ImGui::SameLine();
	if (ImGui::ToolButton(graphics::font_icon_str("rotate"_h).c_str(), tool == ToolRotate))
		tool = ToolRotate;
	ImGui::SameLine();
	if (ImGui::ToolButton(graphics::font_icon_str("down-left-and-up-right-to-center"_h).c_str(), tool == ToolScale))
		tool = ToolScale;
	ImGui::SameLine();
	const char* tool_pivot_names[] = {
		"Individual",
		"Center"
	};
	const char* tool_mode_names[] = {
		"Local",
		"World"
	};
	ImGui::SetNextItemWidth(100.f);
	ImGui::Combo("##pivot", (int*)&tool_pivot, tool_pivot_names, countof(tool_pivot_names));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.f);
	ImGui::Combo("##mode", (int*)&tool_mode, tool_mode_names, countof(tool_mode_names));
	bool* p_snap = nullptr;
	float* p_snap_value = nullptr;
	switch (tool)
	{
	case ToolMove:
		p_snap = &move_snap;
		p_snap_value = last_focused_scene && last_focused_scene->element_targets.empty() ? &move_snap_value : &move_snap_2d_value;
		break;
	case ToolRotate:
		p_snap = &rotate_snap;
		p_snap_value = &rotate_snap_value;
		break;
	case ToolScale:
		p_snap = &scale_snap;
		p_snap_value = &scale_snap_value;
		break;
	}
	ImGui::SameLine();
	if (p_snap)
	{
		ImGui::Checkbox("Snap", p_snap);
		if (*p_snap)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			ImGui::InputFloat("##snap_value", p_snap_value);
		}
	}
	ImGui::SameLine();
	if (ImGui::ToolButton(graphics::font_icon_str("floppy-disk"_h).c_str()))
		app.save_prefab();
	ImGui::SameLine();
	ImGui::Dummy(vec2(0.f, 20.f));

	if (selection.type == Selection::tEntity)
	{
		auto e = selection.as_entity();
		if (auto terrain = e->get_component<cTerrain>(); terrain)
		{
			ImGui::SameLine();
			if (ImGui::ToolButton((graphics::font_icon_str("mound"_h) + "##up").c_str(), tool == ToolTerrainUp))
				tool = ToolTerrainUp;
			ImGui::SameLine();
			if (ImGui::ToolButton((graphics::font_icon_str("mound"_h) + "##down").c_str(), tool == ToolTerrainDown, 180.f))
				tool = ToolTerrainDown;
			ImGui::SameLine();
			if (ImGui::ToolButton(graphics::font_icon_str("paintbrush"_h).c_str(), tool == ToolTerrainPaint))
				tool = ToolTerrainPaint;
		}
		if (auto tile_map = e->get_component<cTileMap>(); tile_map)
		{
			ImGui::SameLine();
			if (ImGui::ToolButton(graphics::font_icon_str("up-long"_h).c_str(), tool == ToolTileMapLevelUp))
				tool = ToolTileMapLevelUp;
			ImGui::SameLine();
			if (ImGui::ToolButton(graphics::font_icon_str("down-long"_h).c_str(), tool == ToolTileMapLevelDown))
				tool = ToolTileMapLevelDown;
			ImGui::SameLine();
			if (ImGui::ToolButton(graphics::font_icon_str("stairs"_h).c_str(), tool == ToolTileMapSlope))
				tool = ToolTileMapSlope;
		}
	}

	ImGui::SameLine();
	ImGui::Dummy(vec2(50.f, 20.f));
	ImGui::SameLine();
	if (!app.e_playing && !app.e_preview)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
		//if (ImGui::ToolButton((graphics::font_icon_str("play"_h) + " Build And Play").c_str()))
		//{
		//	build_project();
		//	add_event([this]() {
		//		cmd_play();
		//		return false;
		//	}, 0.f, 3);
		//}
		//ImGui::SameLine();
		if (ImGui::ToolButton(graphics::font_icon_str("play"_h).c_str()))
			app.cmd_play();
		ImGui::SameLine();
		if (ImGui::ToolButton(graphics::font_icon_str("circle-play"_h).c_str()))
			app.cmd_start_preview(selection.type == Selection::tEntity ? selection.as_entity() : app.e_prefab);
		ImGui::PopStyleColor();
	}
	else
	{
		if (app.e_playing)
		{
			if (!app.paused)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
				ImGui::SameLine();
				if (ImGui::ToolButton(graphics::font_icon_str("pause"_h).c_str()))
					app.cmd_pause();
				ImGui::PopStyleColor();
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
				ImGui::SameLine();
				if (ImGui::ToolButton(graphics::font_icon_str("play"_h).c_str()))
					app.cmd_play();
				ImGui::PopStyleColor();
			}
		}
		else if (app.e_preview)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 1, 1));
			ImGui::SameLine();
			if (ImGui::ToolButton(graphics::font_icon_str("rotate"_h).c_str()))
				app.cmd_restart_preview();
			ImGui::PopStyleColor();
		}
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
		ImGui::SameLine();
		if (ImGui::ToolButton(graphics::font_icon_str("stop"_h).c_str()))
		{
			if (app.e_playing)
				app.cmd_stop();
			else if (app.e_preview)
				app.cmd_stop_preview();
		}
		ImGui::PopStyleColor();
		if (app.e_preview)
		{
			ImGui::SameLine();
			ImGui::Text("[%s]", app.e_preview->name.c_str());
		}
	}

	// toolbar end

	auto render_target_extent = vec2(ImGui::GetContentRegionAvail());
	if (scene_window.fixed_render_target_size)
		render_target_extent = vec2(1280, 720);
	if (!render_target || vec2(render_target->extent) != render_target_extent)
	{
		add_event([this, render_target_extent]() {
			graphics::Queue::get()->wait_idle();
			if (render_target_extent.x > 1 && render_target_extent.y > 1)
			{
				render_target.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(render_target_extent, 1),
					graphics::ImageUsageAttachment | graphics::ImageUsageSampled));
				render_target->filename = L"#scene_render_target";
				render_target->change_layout(graphics::ImageLayoutShaderReadOnly);
				auto iv = render_target->get_view();
				app.renderer->render_tasks.front()->set_targets({ iv });
			}
			else
			{
				render_target.reset();
				app.renderer->render_tasks.front()->set_targets({});
			}
			return false;
		});
	}

	hovering_entity = nullptr;

	if (render_target)
	{
		ImGui::Image(render_target.get(), render_target_extent);
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		app.input->offset = p0;

		bool gizmo_using = false;
		if (use_gizmos)
		{
#if USE_IM_GUIZMO
			if (is_in(tool, ToolMove, ToolScale) && app.e_editor && selection.type == Selection::tEntity)
			{
				node_targets.clear();
				element_targets.clear();
				for (auto e : selection.entities())
				{
					if (auto node = e->get_component<cNode>(); node)
					{
						auto ok = true;
						for (auto t : node_targets)
						{
							if (is_ancestor(t->entity, e))
							{
								ok = false;
								break;
							}
						}
						if (ok)
							node_targets.push_back(node);
					}
					if (auto element = e->get_component<cElement>(); element)
					{
						auto ok = true;
						for (auto t : element_targets)
						{
							if (is_ancestor(t->entity, e))
							{
								ok = false;
								break;
							}
						}
						if (ok)
							element_targets.push_back(element);
					}
				}

				static bool last_gizmo_using = false;
				auto gizmo_manipulate = [this](const mat4& view_mat, const mat4& proj_mat, mat4& mat) {
					auto op = ImGuizmo::TRANSLATE;
					auto mode = ImGuizmo::LOCAL;
					vec3 snap_value; float* p_snap_value = nullptr;
					switch (tool)
					{
					case ToolMove:
						op = ImGuizmo::TRANSLATE;
						if (move_snap)
						{
							snap_value = vec3(element_targets.empty() ? move_snap_value : move_snap_2d_value);
							p_snap_value = &snap_value[0];
						}
						break;
					case ToolRotate:
						op = ImGuizmo::ROTATE;
						if (rotate_snap)
						{
							snap_value = vec3(rotate_snap_value);
							p_snap_value = &snap_value[0];
						}
						break;
					case ToolScale:
						op = ImGuizmo::SCALE;
						if (scale_snap)
						{
							snap_value = vec3(scale_snap_value);
							p_snap_value = &snap_value[0];
						}
						break;
					}
					switch (tool_mode)
					{
					case ToolLocal:
						mode = ImGuizmo::LOCAL;
						break;
					case ToolWorld:
						mode = ImGuizmo::WORLD;
						break;
					}
					ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
					return ImGuizmo::Manipulate(&view_mat[0][0], &proj_mat[0][0], op, mode, &mat[0][0], nullptr, p_snap_value);
					};
				if (!node_targets.empty() && element_targets.empty())
				{
					static std::vector<vec3> before_editing_poses;
					static std::vector<quat> before_editing_quts;
					static std::vector<vec3> before_editing_scls;
					static vec3 center;
					ImGuizmo::BeginFrame();
					ImGuizmo::SetRect(p0.x, p0.y, p1.x - p0.x, p1.y - p0.y);
					auto matp = camera->proj_mat; matp[1][1] *= -1.f;
					mat4 mat;
					if (node_targets.size() > 1)
					{
						center = vec3(0.f);
						for (auto t : node_targets)
							center += t->global_pos();
						center /= (float)node_targets.size();
						mat = translate(mat4(1.f), center);
					}
					else
						mat = node_targets[0]->transform;
					auto changed = gizmo_manipulate(camera->view_mat, matp, mat);
					gizmo_using = ImGuizmo::IsUsing();
					if (!last_gizmo_using && gizmo_using)
					{
						before_editing_poses.clear();
						before_editing_quts.clear();
						before_editing_scls.clear();
						switch (tool)
						{
						case ToolMove:
							for (auto t : node_targets)
								before_editing_poses.push_back(t->pos);
							break;
						case ToolRotate:
							for (auto t : node_targets)
								before_editing_quts.push_back(t->qut);
							if (node_targets.size() > 1 && tool_pivot == ToolCenter)
							{
								for (auto t : node_targets)
									before_editing_poses.push_back(t->pos);
							}
							break;
						case ToolScale:
							for (auto t : node_targets)
								before_editing_scls.push_back(t->scl);
							break;
						}
					}
					if (changed)
					{
						if (node_targets.size() == 1)
						{
							if (auto pnode = node_targets[0]->entity->get_parent_component<cNodeT>(); pnode)
								mat = inverse(pnode->transform) * mat;
						}
						vec3 pos; quat qut; vec3 scl; vec3 skew; vec4 perspective;
						decompose(mat, scl, qut, pos, skew, perspective);
						if (tool == ToolMove)
						{
							if (node_targets.size() > 1)
							{
								auto diff = pos - center;
								for (auto t : node_targets)
								{
									t->add_pos(diff);
									if (auto ins = get_root_prefab_instance(t->entity); ins)
										ins->mark_modification(node_targets[0]->entity->file_id.to_string() + "|flame::cNode|pos");
								}
							}
							else
							{
								node_targets[0]->set_pos(pos);
								if (auto ins = get_root_prefab_instance(node_targets[0]->entity); ins)
									ins->mark_modification(node_targets[0]->entity->file_id.to_string() + "|flame::cNode|pos");
							}
						}
						if (tool == ToolRotate)
						{
							if (node_targets.size() > 1)
							{
								for (auto t : node_targets)
								{
									t->mul_qut(qut);
									if (auto ins = get_root_prefab_instance(t->entity); ins)
										ins->mark_modification(t->entity->file_id.to_string() + "|flame::cNode|qut");
									if (tool_pivot == ToolCenter)
									{
										t->set_pos(center + qut * (t->global_pos() - center));
										if (auto ins = get_root_prefab_instance(t->entity); ins)
											ins->mark_modification(t->entity->file_id.to_string() + "|flame::cNode|pos");
									}
								}
							}
							else
							{
								node_targets[0]->set_qut(qut);
								if (auto ins = get_root_prefab_instance(node_targets[0]->entity); ins)
									ins->mark_modification(node_targets[0]->entity->file_id.to_string() + "|flame::cNode|qut");
							}
						}
						if (tool == ToolScale)
						{
							if (node_targets.size() > 1)
							{

							}
							else
							{
								node_targets[0]->set_scl(scl);
								if (auto ins = get_root_prefab_instance(node_targets[0]->entity); ins)
									ins->mark_modification(node_targets[0]->entity->file_id.to_string() + "|flame::cNode|scl");
							}
						}
					}
					if (last_gizmo_using && !gizmo_using)
					{
						std::vector<GUID> ids;
						for (auto t : node_targets)
							ids.push_back(t->entity->instance_id);
						if (tool == ToolMove)
						{
							std::vector<std::string> old_values(ids.size());
							std::vector<std::string> new_values(ids.size());
							for (auto i = 0; i < ids.size(); i++)
								old_values[i] = str(before_editing_poses[i]);
							for (auto i = 0; i < ids.size(); i++)
								new_values[i] = str(node_targets[i]->pos);
							add_history(new EntityModifyHistory(ids, "flame::cNode"_h, "pos"_h, old_values, new_values));
							app.prefab_unsaved = true;
						}
						if (tool == ToolRotate)
						{
							std::vector<std::string> old_values(ids.size());
							std::vector<std::string> new_values(ids.size());
							for (auto i = 0; i < old_values.size(); i++)
								old_values[i] = str((vec4&)before_editing_quts[i]);
							for (auto i = 0; i < ids.size(); i++)
								new_values[i] = str((vec4&)node_targets[i]->qut);
							add_history(new EntityModifyHistory(ids, "flame::cNode"_h, "qut"_h, old_values, new_values));
							if (node_targets.size() > 1 && tool_pivot == ToolCenter)
							{
								std::vector<std::string> old_values(ids.size());
								std::vector<std::string> new_values(ids.size());
								for (auto i = 0; i < ids.size(); i++)
									old_values[i] = str(before_editing_poses[i]);
								for (auto i = 0; i < ids.size(); i++)
									new_values[i] = str(node_targets[i]->pos);
								add_history(new EntityModifyHistory(ids, "flame::cNode"_h, "pos"_h, old_values, new_values));
							}
							app.prefab_unsaved = true;
						}
						if (tool == ToolScale)
						{
							std::vector<std::string> old_values(ids.size());
							std::vector<std::string> new_values(ids.size());
							for (auto i = 0; i < ids.size(); i++)
								old_values[i] = str(before_editing_scls[i]);
							for (auto i = 0; i < ids.size(); i++)
								new_values[i] = str(node_targets[i]->scl);
							add_history(new EntityModifyHistory(ids, "flame::cNode"_h, "scl"_h, old_values, new_values));
							app.prefab_unsaved = true;
						}
					}
					last_gizmo_using = gizmo_using;
				}
				else if (node_targets.empty() && !element_targets.empty())
				{
					static vec2 center;
					ImGuizmo::BeginFrame();
					ImGuizmo::SetRect(p0.x, p0.y, p1.x - p0.x, p1.y - p0.y);
					static auto matv = mat4(1.f);
					auto		matp = mat4(1.f);
					matp[1][1] *= -1.f;
					matp = translate(matp, vec3(-1.f, -1.f, 0.5f));
					matp = scale(matp, vec3(2.f / render_target_extent, 1.f));
					mat4 mat = mat4(1.f);
					if (element_targets.size() > 1)
					{
						if (!last_gizmo_using)
						{
							center = vec3(0.f);
							for (auto t : element_targets)
								center += t->global_pos0();
							center /= (float)element_targets.size();
						}
						mat = translate(mat, vec3(center, 0.f));
					}
					else
					{
						vec2 pos;
						if (app.input->kbtn[Keyboard_Alt])
							pos = element_targets[0]->global_pos1();
						else
							pos = element_targets[0]->global_pos0();
						mat = translate(mat, vec3(pos, 0.f));
					}

					auto changed = gizmo_manipulate(matv, matp, mat);
					if (changed)
					{
						auto m3 = mat3(1.f);
						m3[0] = mat[0]; m3[1] = mat[1];
						m3[2] = mat[3]; m3[2][2] = 1.f;
						if (element_targets.size() == 1)
						{
							if (auto pelement = element_targets[0]->entity->get_parent_component<cElementT>(); pelement)
								m3 = inverse(mat3(pelement->transform)) * m3;
						}
						vec2 pos; vec2 scl;
						pos = m3[2];
						scl.x = length(vec2(m3[0]));
						scl.y = length(vec2(m3[1]));
						if (tool == ToolMove)
						{
							if (element_targets.size() > 1)
							{
								auto diff = pos - element_targets[0]->pos;
								for (auto t : element_targets)
								{
									t->add_pos(diff);
									if (auto ins = get_root_prefab_instance(t->entity); ins)
										ins->mark_modification(element_targets[0]->entity->file_id.to_string() + "|flame::cElement|pos");
								}
							}
							else
							{
								if (app.input->kbtn[Keyboard_Alt])
									element_targets[0]->set_ext(pos - element_targets[0]->pos);
								else
									element_targets[0]->set_pos(pos);
								if (auto ins = get_root_prefab_instance(element_targets[0]->entity); ins)
									ins->mark_modification(element_targets[0]->entity->file_id.to_string() + "|flame::cElement|pos");
							}
						}
						if (tool == ToolScale)
						{
							if (element_targets.size() > 1)
							{

							}
							else
							{
								element_targets[0]->set_scl(scl);
								if (auto ins = get_root_prefab_instance(element_targets[0]->entity); ins)
									ins->mark_modification(element_targets[0]->entity->file_id.to_string() + "|flame::cElement|scl");
							}
						}
					}
					gizmo_using = ImGuizmo::IsUsing();
					last_gizmo_using = gizmo_using;
				}
			}
#endif
		}

		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		if (use_gizmos)
		{
			if (ImGui::IsWindowHovered())
			{
				auto camera_node = camera->node;

				auto get_tar = [&]() {
					return camera_node->global_pos() - camera_node->z_axis() * camera_zoom;
				};

				if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
				{
					disp /= vec2(render_target->extent);
					if (!io.KeyAlt)
					{
						if (io.MouseDown[ImGuiMouseButton_Middle])
						{
							camera_node->add_pos((-camera_node->x_axis() * disp.x +
								camera_node->y_axis() * disp.y) * camera_zoom);
						}
						else if (io.MouseDown[ImGuiMouseButton_Right])
						{
							disp *= -180.f;
							disp = radians(disp);
							auto qut = angleAxis(disp.x, vec3(0.f, 1.f, 0.f)) * camera_node->qut;
							qut = angleAxis(disp.y, qut * vec3(1.f, 0.f, 0.f)) * qut;
							camera_node->set_qut(qut);
						}
					}
					else
					{
						if (io.MouseDown[ImGuiMouseButton_Left])
						{
							disp *= -180.f;
							disp = radians(disp);
							auto qut = angleAxis(disp.x, vec3(0.f, 1.f, 0.f)) * camera_node->qut;
							qut = angleAxis(disp.y, qut * vec3(1.f, 0.f, 0.f)) * qut;
							camera_node->set_qut(qut);
							camera_node->set_pos(get_tar() + (qut * vec3(0.f, 0.f, 1.f)) * camera_zoom);
						}
					}
				}
				{
					static vec2 last_mpos = vec2(0.f);
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
						last_mpos = io.MousePos;
					if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle) && (vec2)io.MousePos == last_mpos)
						camera_node->set_pos(hovering_pos + camera_node->z_axis() * camera_zoom);
				}
				if (auto scroll = io.MouseWheel; scroll != 0.f)
				{
					auto tar = get_tar();
					if (scroll < 0.f)
						camera_zoom = camera_zoom * 1.1f + 0.5f;
					else
						camera_zoom = max(0.f, camera_zoom / 1.1f - 0.5f);
					camera_node->set_pos(tar + camera_node->z_axis() * camera_zoom);
				}
				if (!io.WantCaptureKeyboard)
				{
					if (!io.KeysDown[Keyboard_Ctrl] && !io.KeysDown[Keyboard_Alt] && !io.KeysDown[Keyboard_Shift])
					{
						if (io.MouseDown[ImGuiMouseButton_Right])
						{
							if (io.KeysDown[Keyboard_W])
							{
								camera_node->add_pos(-camera_node->z_axis() * 0.2f);
								app.render_frames += 30;
							}
							if (io.KeysDown[Keyboard_S])
							{
								camera_node->add_pos(+camera_node->z_axis() * 0.2f);
								app.render_frames += 30;
							}
							if (io.KeysDown[Keyboard_A])
							{
								camera_node->add_pos(-camera_node->x_axis() * 0.2f);
								app.render_frames += 30;
							}
							if (io.KeysDown[Keyboard_D])
							{
								camera_node->add_pos(+camera_node->x_axis() * 0.2f);
								app.render_frames += 30;
							}
						}
						else
						{
							if (ImGui::IsKeyPressed(Keyboard_Q))
								tool = ToolSelect;
							if (ImGui::IsKeyPressed(Keyboard_W))
								tool = ToolMove;
							if (ImGui::IsKeyPressed(Keyboard_E))
								tool = ToolRotate;
							if (ImGui::IsKeyPressed(Keyboard_R))
								tool = ToolScale;
						}
						if (ImGui::IsKeyPressed(Keyboard_Del))
							app.cmd_delete_entities(selection.get_entities());
					}
					if (!io.KeysDown[Keyboard_Ctrl] && !ImGui::IsKeyDown(Keyboard_Shift) && io.KeysDown[Keyboard_F])
						focus_to_selected();
					if (!io.KeysDown[Keyboard_Ctrl] && !io.KeysDown[Keyboard_Alt] && ImGui::IsKeyDown(Keyboard_Shift) && ImGui::IsKeyPressed(Keyboard_D))
						app.cmd_duplicate_entities(selection.get_entities());
					if (!io.KeysDown[Keyboard_Ctrl] && !io.KeysDown[Keyboard_Alt] && io.KeysDown[Keyboard_Shift])
					{
						if (io.KeysDown[Keyboard_G])
							selected_to_focus();
					}
					if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_S))
						app.save_prefab();
				}

				if (auto hovering_element = sRenderer::instance()->pick_up_2d(app.input->mpos);
					hovering_element && hovering_element->entity != sScene::instance()->first_element)
				{
					hovering_entity = hovering_element->entity;
					hovering_pos = vec3(app.input->mpos, 0.f);
				}

				if (!hovering_entity)
				{
					auto hovering_node = sRenderer::instance()->pick_up(app.input->mpos, &hovering_pos, [](cNodePtr n, DrawData& draw_data) {
						if (draw_data.categories & CateMesh)
						{
							if (auto mesh = n->entity->get_component<cMesh>(); mesh)
							{
								if (mesh->instance_id != -1 && mesh->mesh_res_id != -1 && mesh->material_res_id != -1)
									draw_data.meshes.emplace_back(mesh->instance_id, mesh->mesh_res_id, mesh->material_res_id);
							}
						}
						if (draw_data.categories & CateTerrain)
						{
							if (auto terrain = n->entity->get_component<cTerrain>(); terrain)
								draw_data.terrains.emplace_back(terrain->instance_id, terrain->blocks, terrain->material_res_id);
						}
						if (draw_data.categories & CateMarchingCubes)
						{
							if (auto volume = n->entity->get_component<cVolume>(); volume && volume->marching_cubes)
								draw_data.volumes.emplace_back(volume->instance_id, volume->blocks, volume->material_res_id);
						}
					});

					if (hovering_node)
						hovering_entity = hovering_node->entity;
				}

				if (!gizmo_using && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyAlt)
				{
					auto get_top_entity = [](EntityPtr e) {
						if (auto ins = get_root_prefab_instance(e); ins)
						{
							if (!selection.selecting(ins->e))
								return ins->e;
						}
						return e;
					};
					if (ImGui::IsKeyDown(Keyboard_Ctrl))
					{
						if (hovering_entity)
						{
							auto e = get_top_entity(hovering_entity);
							auto entities = selection.get_entities();
							auto found = false;
							for (auto it = entities.begin(); it != entities.end();)
							{
								if (*it == e)
								{
									found = true;
									it = entities.erase(it);
									break;
								}
								else
									it++;
							}
							if (!found)
								entities.push_back(e);
							selection.select(entities, "scene"_h);
						}
					}
					else if (ImGui::IsKeyDown(Keyboard_Shift))
					{
						if (hovering_entity)
						{
							auto e = get_top_entity(hovering_entity);
							auto entities = selection.get_entities();
							auto found = false;
							for (auto e2 : entities)
							{
								if (e2 == e)
								{
									found = true;
									break;
								}
							}
							if (!found)
								entities.push_back(e);
							selection.select(entities, "scene"_h);
						}
					}
					else
					{
						if (hovering_entity)
							selection.select(get_top_entity(hovering_entity), "scene"_h);
						else
							selection.clear("scene"_h);
					}
				}

				{
					auto s = str(hovering_pos);
					auto sz = ImGui::CalcTextSize(s.c_str(), s.c_str() + s.size());
					ImGui::GetWindowDrawList()->AddRectFilled(p0, (vec2)p0 + (vec2)sz, ImColor(0.f, 0.f, 0.f, 0.5f));
					ImGui::GetWindowDrawList()->AddText(p0, ImColor(255.f, 255.f, 255.f), s.c_str(), s.c_str() + s.size());
				}
			}
		}

		if (show_outline)
		{
			auto outline_node = [&](EntityPtr e, const cvec4& col) {
				if (!e->global_enable)
					return;
				if (auto node = e->get_component<cNode>(); !node || !AABB_frustum_check(camera->frustum, node->bounds))
					return;
				if (auto mesh = e->get_component<cMesh>(); mesh && mesh->instance_id != -1 && mesh->mesh_res_id != -1)
				{
					ObjectDrawData d("mesh"_h, mesh->mesh_res_id, mesh->instance_id);
					sRenderer::instance()->draw_outlines({ d }, col, 1, OutlineMax);
				}
				if (auto terrain = e->get_component<cTerrain>(); terrain && terrain->instance_id != -1 && terrain->height_map)
				{
					ObjectDrawData d("terrain"_h, 0, terrain->instance_id);
					sRenderer::instance()->draw_outlines({ d }, col, 1, OutlineMax);
				}
				if (auto armature = e->get_component<cArmature>(); armature && armature->model)
				{
					std::vector<ObjectDrawData> ds;
					for (auto& c : e->children)
					{
						if (auto mesh = c->get_component<cMesh>(); mesh && mesh->instance_id != -1 && mesh->mesh_res_id != -1)
							ds.emplace_back("mesh"_h, mesh->mesh_res_id, mesh->instance_id);
					}
					sRenderer::instance()->draw_outlines(ds, col, 1, OutlineMax);
				}
			};
			bool already_outline_hovering = false;
			for (auto e : selection.entities())
			{
				if (e == hovering_entity)
					already_outline_hovering = true;
				outline_node(e, cvec4(200, 200, 100, 255));
			}
			if (!already_outline_hovering && hovering_entity)
				outline_node(hovering_entity, cvec4(128, 128, 64, 255));
		}
		if (show_AABB)
		{
			auto draw_aabb = [](EntityPtr e) {
				if (auto node = e->get_component<cNode>(); node)
				{
					if (!node->bounds.invalid())
					{
						auto points = node->bounds.get_points();
						auto line_pts = Frustum::points_to_lines(points.data());
						sRenderer::instance()->draw_primitives(PrimitiveLineList, line_pts.data(), line_pts.size(), cvec4(255, 127, 127, 255));
					}
				}
			};
			if (show_AABB_only_selected)
			{
				if (selection.type == Selection::tEntity)
				{
					for (auto i = 0; i < selection.objects.size(); i++)
					{
						auto e = selection.as_entity(i);
						if (!e->global_enable)
							break;
						draw_aabb(e);
					}
				}
			}
			else
			{
				World::instance()->root->forward_traversal([&](EntityPtr e) {
					if (!e->global_enable)
						return false;
					draw_aabb(e);
					return true;
				});
			}
		}
		if (show_axis)
		{
			if (selection.type == Selection::tEntity)
			{
				auto e = selection.as_entity();
				if (e->global_enable)
				{
					if (auto node = e->get_component<cNode>(); node)
					{
						vec3 line_pts[2];
						line_pts[0] = node->global_pos(); line_pts[1] = node->global_pos() + node->x_axis();
						sRenderer::instance()->draw_primitives(PrimitiveLineList, line_pts, 2, cvec4(255, 0, 0, 255));
						line_pts[0] = node->global_pos(); line_pts[1] = node->global_pos() + node->y_axis();
						sRenderer::instance()->draw_primitives(PrimitiveLineList, line_pts, 2, cvec4(0, 255, 0, 255));
						line_pts[0] = node->global_pos(); line_pts[1] = node->global_pos() + node->z_axis();
						sRenderer::instance()->draw_primitives(PrimitiveLineList, line_pts, 2, cvec4(0, 0, 255, 255));
					}
				}
			}
		}
		if (show_bones)
		{
			World::instance()->root->forward_traversal([](EntityPtr e) {
				if (!e->global_enable)
					return false;
				if (auto arm = e->get_component<cArmature>(); arm)
				{
					std::function<void(cNodePtr)> draw_node;
					draw_node = [&](cNodePtr n) {
						vec3 line_pts[2];
						line_pts[0] = n->global_pos();
						for (auto& c : n->entity->children)
						{
							auto nn = c->get_component<cNode>();
							if (nn)
							{
								line_pts[1] = nn->global_pos();
								sRenderer::instance()->draw_primitives(PrimitiveLineList, line_pts, 2, cvec4(255));
								draw_node(nn);
							}
						}
					};
					draw_node(arm->node);
				}
				return true;
			});
		}
		if (show_navigation || show_navigation_frames)
		{
			World::instance()->root->forward_traversal([](EntityPtr e) {
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
					sRenderer::instance()->draw_primitives(PrimitiveLineList, pts.data(), (uint)pts.size(), cvec4(127, 0, 255, 255));
					center.y += h;
					for (auto i = 0; i < n; i++)
					{
						pts[i * 2 + 0] = center + vec3(r * circle_pts[i + 0], 0.f).xzy();
						pts[i * 2 + 1] = center + vec3(r * circle_pts[i + 1], 0.f).xzy();
					}
					sRenderer::instance()->draw_primitives(PrimitiveLineList, pts.data(), (uint)pts.size(), cvec4(127, 0, 255, 255));
					center = p;
					pts[0] = center + r * vec3(+1.f, 0.f, 0.f);
					pts[1] = pts[0] + vec3(0.f, h, 0.f);
					sRenderer::instance()->draw_primitives(PrimitiveLineList, pts.data(), 2, cvec4(127, 0, 255, 255));
					pts[0] = center + r * vec3(-1.f, 0.f, 0.f);
					pts[1] = pts[0] + vec3(0.f, h, 0.f);
					sRenderer::instance()->draw_primitives(PrimitiveLineList, pts.data(), 2, cvec4(127, 0, 255, 255));
					pts[0] = center + r * vec3(0.f, 0.f, +1.f);
					pts[1] = pts[0] + vec3(0.f, h, 0.f);
					sRenderer::instance()->draw_primitives(PrimitiveLineList, pts.data(), 2, cvec4(127, 0, 255, 255));
					pts[0] = center + r * vec3(0.f, 0.f, -1.f);
					pts[1] = pts[0] + vec3(0.f, h, 0.f);
					sRenderer::instance()->draw_primitives(PrimitiveLineList, pts.data(), 2, cvec4(127, 0, 255, 255));
				};
				if (auto agent = e->get_component<cNavAgent>(); agent)
					draw_cylinder(agent->node->global_pos(), agent->radius, agent->height);
				if (auto obstacle = e->get_component<cNavObstacle>(); obstacle)
					draw_cylinder(obstacle->node->global_pos(), obstacle->radius, obstacle->height);
				return true;
			});

			sScene::instance()->draw_debug_primitives();

			if (show_navigation_frames > 0)
				show_navigation_frames--;
		}

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && selection.type == Selection::tEntity)
		{
			auto e = selection.as_entity();
			if (auto terrain = e->get_component<cTerrain>(); terrain)
			{
			}
			if (auto tile_map = e->get_component<cTileMap>(); tile_map)
			{
				tile_map_editing();
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
							if (auto node = e->get_component<cNode>(); node)
							{
								auto pos = hovering_pos;
								if (!hovering_entity)
								{
									auto camera_node = curr_camera()->node;
									auto camera_pos = camera_node->global_pos();
									auto v = normalize(pos - camera_pos);
									pos = camera_pos + v * (camera_zoom / dot(v, -camera_node->z_axis()));
								}
								node->set_pos(get_snap_pos(pos));
							}
							if (app.e_playing)
								app.e_playing->add_child(e);
							else
								app.e_prefab->add_child(e);
							app.prefab_unsaved = true;
							return false;
						});
					}
					else if (ext == L".fmat")
					{
						if (hovering_entity)
						{
							if (auto mesh = hovering_entity->get_component<cMesh>(); mesh)
							{
								mesh->set_material_name(path);
								app.prefab_unsaved = true;
							}
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::End();
	if (!opened)
		delete this;
}

SceneWindow::SceneWindow() :
	Window("Scene")
{
}

View* SceneWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new SceneView;
	return nullptr;
}

View* SceneWindow::open_view(const std::string& name)
{
	return new SceneView(name);
}

SceneView* SceneWindow::first_view() const
{
	return views.empty() ? nullptr : (SceneView*)views.front().get();
}

