#include "app.h"
#include "selection.h"
#include "history.h"
#include "scene_window.h"
#include "project_window.h"
#include "sheet_window.h"
#include "blueprint_window.h"
#include "inspector_window.h"

#include <flame/xml.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo_serialize.h>
#include <flame/foundation/sheet.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/model.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/debug.h>
#include <flame/universe/draw_data.h>
#include <flame/universe/timeline.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/directional_light.h>
#include <flame/universe/systems/renderer.h>
#include <flame/universe/systems/graveyard.h>

std::vector<Window*> windows;

void View::title_context_menu()
{
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
	{
		const auto im_wnd = ImGui::GetCurrentWindow();
		ImRect rect = im_wnd->DockIsActive ? im_wnd->DockTabItemRect : im_wnd->TitleBarRect();
		if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max, false))
			ImGui::OpenPopup("title_context_menu");
	}
	if (ImGui::BeginPopup("title_context_menu"))
	{
		if (ImGui::Selectable("New Tab"))
			window->open_view(true);
		ImGui::EndPopup();
	}
}

App app;

struct IntelliSense
{
	std::vector<std::pair<std::string, TypeInfo*>> top_using_types;

	void collect()
	{
		top_using_types.clear();

		std::map<TypeInfo*, int> type_count;

		auto assets_path = app.project_path / L"assets";
		for (auto& it : std::filesystem::recursive_directory_iterator(assets_path))
		{
			if (it.is_regular_file() && it.path().extension() == L".bp")
			{
				if (auto bp = Blueprint::get(it.path()); bp)
				{
					for (auto& v : bp->variables)
					{
						if (type_count.find(v.type) == type_count.end())
							type_count[v.type] = 1;
						else
							type_count[v.type]++;
					}
					for (auto& g : bp->groups)
					{
						for (auto& v : g->variables)
						{
							if (type_count.find(v.type) == type_count.end())
								type_count[v.type] = 1;
							else
								type_count[v.type]++;
						}
					}
					Blueprint::release(bp);
				}
			}
		}

		std::vector<std::pair<TypeInfo*, int>> type_count_vec;
		for (auto& it : type_count)
			type_count_vec.push_back(it);
		std::sort(type_count_vec.begin(), type_count_vec.end(), [](auto& a, auto& b) {
			return a.second > b.second;
		});
		top_using_types.resize(min(5, (int)type_count_vec.size()));
		for (auto i = 0; i < top_using_types.size(); i++)
			top_using_types[i] = { type_count_vec[i].first->name, type_count_vec[i].first };
	}
}intelli_sense;

struct Preferences
{
	bool use_flame_debugger = false; // use flame visual studio project debugger or use opened project one

};
static Preferences preferences;

static std::filesystem::path preferences_path = L"preferences.ini";

bool filter_name(const std::string& name, const std::string& find_str, bool match_case, bool match_whole_word)
{
	bool ok;
	if (match_whole_word)
	{
		if (match_case)
			ok = name == find_str;
		else
			ok = SUS::match_case_insensitive(name, find_str);
	}
	else
	{
		if (match_case)
			ok = name.find(find_str) != std::string::npos;
		else
			ok = SUS::find_case_insensitive(name, find_str);
	}
	return ok;
}

std::string get_value_str(TypeInfo* type, void* data)
{
	auto show_entity = [](EntityPtr entity)->std::string {
		if (entity && entity != INVALID_POINTER && entity != (EntityPtr)0xCDCDCDCDCDCDCDCD)
		{
			if (World::instance()->root->has_child_recursively(entity))
			{
				auto node = entity->get_component<cNode>();
				return std::format("\nName: {}\nPos: {}\nParent: {}\n", entity->name,
					node ? str(node->global_pos()) : "N\\A", entity->parent ? entity->parent->name : "[None]");
			}
		}
		return "\nInvalid Entity\n";
	};
	if (type->tag == TagD || is_pointer(type->tag))
	{
		auto ret = type->serialize(data);
		if (type == TypeInfo::get<EntityPtr>())
			ret += show_entity(*(EntityPtr*)data);
		return ret;
	}
	else if (type->tag == TagU)
	{
		auto ui = ((TypeInfo_Udt*)type)->ui;
		if (ui)
		{
			std::string ret;
			ret += '\n';
			for (auto& vi : ui->variables)
			{
				ret += std::format(".{}={}", vi.name, vi.type->serialize((char*)data + vi.offset));
				ret += '\n';
			}
			return ret;
		}
	}
	else if (is_vector(type->tag))
	{
		auto item_type = type->get_wrapped();
		auto parray = (std::vector<char>*)data;
		auto array_size = (uint)parray->size() / item_type->size;
		auto ret = std::format("Size: {}\n", array_size);
		for (auto i = 0; i < array_size; i++)
		{
			auto item = parray->data() + i * item_type->size;
			ret += std::format("\n[{}]: {}", i, item_type->serialize(item));
			if (item_type == TypeInfo::get<EntityPtr>())
				ret += show_entity(*(EntityPtr*)item);
		}
		return ret;
	}
	return "";
}

void show_entities_menu()
{
	if (ImGui::MenuItem("New Empty"))
		app.cmd_new_entities(selection.get_entities());
	if (ImGui::BeginMenu("New 3D"))
	{
		if (ImGui::MenuItem("Node"))
			app.cmd_new_entities(selection.get_entities(), "node"_h);
		if (ImGui::MenuItem("Plane"))
			app.cmd_new_entities(selection.get_entities(), "plane"_h);
		if (ImGui::MenuItem("Cube"))
			app.cmd_new_entities(selection.get_entities(), "cube"_h);
		if (ImGui::MenuItem("Sphere"))
			app.cmd_new_entities(selection.get_entities(), "sphere"_h);
		if (ImGui::MenuItem("Cylinder"))
			app.cmd_new_entities(selection.get_entities(), "cylinder"_h);
		if (ImGui::MenuItem("Triangular Prism"))
			app.cmd_new_entities(selection.get_entities(), "tri_prism"_h);
		if (ImGui::MenuItem("Directional Light"))
			app.cmd_new_entities(selection.get_entities(), "dir_light"_h);
		if (ImGui::MenuItem("Point Light"))
			app.cmd_new_entities(selection.get_entities(), "pt_light"_h);
		if (ImGui::MenuItem("Camera"))
			app.cmd_new_entities(selection.get_entities(), "camera"_h);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("New 2D"))
	{
		if (ImGui::MenuItem("Element"))
			app.cmd_new_entities(selection.get_entities(), "element"_h);
		if (ImGui::MenuItem("Image"))
			app.cmd_new_entities(selection.get_entities(), "image"_h);
		if (ImGui::MenuItem("Text"))
			app.cmd_new_entities(selection.get_entities(), "text"_h);
		if (ImGui::MenuItem("Layout"))
			app.cmd_new_entities(selection.get_entities(), "layout"_h);
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Duplicate (Shift+D)"))
		app.cmd_duplicate_entities(selection.get_entities());
	if (ImGui::MenuItem("Delete (Del)"))
		app.cmd_delete_entities(selection.get_entities());
}

TypeInfo* show_types_menu()
{
	TypeInfo* ret = nullptr;

	static std::string type_filter = "";
	static bool filter_match_case = false;
	static bool filter_match_whole_word = false;
	ImGui::InputText("##Filter", &type_filter);
	ImGui::SameLine();
	ImGui::ToolButton("C", &filter_match_case);
	ImGui::SameLine();
	ImGui::ToolButton("W", &filter_match_whole_word);
	ImGui::Separator();

	if (!intelli_sense.top_using_types.empty())
	{
		for (auto& ti : intelli_sense.top_using_types)
		{
			if (!type_filter.empty())
			{
				if (!filter_name(ti.first, type_filter, filter_match_case, filter_match_whole_word))
					continue;
			}
			if (ImGui::Selectable(ti.first.c_str()))
				ret = ti.second;
		}
		ImGui::Separator();
	}

	if (ImGui::BeginMenu("Enum"))
	{
		for (auto& ei : tidb.enums)
		{
			if (!type_filter.empty())
			{
				if (!filter_name(ei.second.name, type_filter, filter_match_case, filter_match_whole_word))
					continue;
			}
			if (ImGui::Selectable(ei.second.name.c_str()))
				ret = TypeInfo::get(TagE, ei.second.name, tidb);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Data"))
	{
		for (auto bt : tidb.basic_types)
		{
			if (!type_filter.empty())
			{
				if (!SUS::find_case_insensitive(bt->name, type_filter))
					continue;
			}
			if (ImGui::Selectable(bt->name.c_str()))
				ret = TypeInfo::get(TagD, bt->name, tidb);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("UDT"))
	{
		for (auto& ui : tidb.udts)
		{
			if (!type_filter.empty())
			{
				if (!SUS::find_case_insensitive(ui.second.name, type_filter))
					continue;
			}
			if (ImGui::Selectable(ui.second.name.c_str()))
				ret = TypeInfo::get(TagU, ui.second.name, tidb);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Pointer"))
	{
		if (ImGui::BeginMenu("Of Enum"))
		{
			for (auto& ei : tidb.enums)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(ei.second.name, type_filter))
						continue;
				}
				if (ImGui::Selectable(ei.second.name.c_str()))
					ret = TypeInfo::get(TagPE, ei.second.name, tidb);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Of Data"))
		{
			for (auto bt : tidb.basic_types)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(bt->name, type_filter))
						continue;
				}
				if (ImGui::Selectable(bt->name.c_str()))
					ret = TypeInfo::get(TagPD, bt->name, tidb);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Of Udt"))
		{
			for (auto& ui : tidb.udts)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(ui.second.name, type_filter))
						continue;
				}
				if (ImGui::Selectable(ui.second.name.c_str()))
					ret = TypeInfo::get(TagPU, ui.second.name, tidb);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Unique Pointer"))
	{
		if (ImGui::BeginMenu("Of Udt"))
		{
			for (auto& ui : tidb.udts)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(ui.second.name, type_filter))
						continue;
				}
				if (ImGui::Selectable(ui.second.name.c_str()))
					ret = TypeInfo::get(TagQU, ui.second.name, tidb);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Vector"))
	{
		if (ImGui::BeginMenu("Of Enum"))
		{
			for (auto& ei : tidb.enums)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(ei.second.name, type_filter))
						continue;
				}
				if (ImGui::Selectable(ei.second.name.c_str()))
					ret = TypeInfo::get(TagVE, ei.second.name, tidb);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Of Data"))
		{
			for (auto bt : tidb.basic_types)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(bt->name, type_filter))
						continue;
				}
				if (ImGui::Selectable(bt->name.c_str()))
					ret = TypeInfo::get(TagVD, bt->name, tidb);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Of Udt"))
		{
			for (auto& ui : tidb.udts)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(ui.second.name, type_filter))
						continue;
				}
				if (ImGui::Selectable(ui.second.name.c_str()))
					ret = TypeInfo::get(TagVU, ui.second.name, tidb);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Of Pointer Of Udt"))
		{
			for (auto& ui : tidb.udts)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(ui.second.name, type_filter))
						continue;
				}
				if (ImGui::Selectable(ui.second.name.c_str()))
					ret = TypeInfo::get(TagVPU, ui.second.name, tidb);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Of Unique Pointer Of Udt"))
		{
			for (auto& ui : tidb.udts)
			{
				if (!type_filter.empty())
				{
					if (!SUS::find_case_insensitive(ui.second.name, type_filter))
						continue;
				}
				if (ImGui::Selectable(ui.second.name.c_str()))
					ret = TypeInfo::get(TagVQU, ui.second.name, tidb);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ret)
		type_filter.clear();
	return ret;
}

void open_message_dialog(const std::string& title, const std::string& message)
{
	if (title == "[RestructurePrefabInstanceWarnning]")
	{
		ImGui::OpenMessageDialog("Cannot restructure Prefab Instance",
			"You cannot remove/reposition entities in a Prefab Instance\n"
			"And added entities must be at the end\n"
			"Edit it in that prefab");
	}
	else
		ImGui::OpenMessageDialog(title, message);
}

void view_image(graphics::ImagePtr image, int* view_swizzle, int* view_sampler, int* view_level, int* view_layer, float* view_zoom, float* view_range_min, float* view_range_max)
{
	ImGui::Text("Format: %s", TypeInfo::serialize_t(image->format).c_str());
	ImGui::SameLine();
	ImGui::Text("Extent: %s", str(image->extent).c_str());
	ImGui::SameLine();
	ImGui::Text("Usage: %s", TypeInfo::serialize_t(image->usage).c_str());

	ImGui::PushItemWidth(100.f);
	static const char* swizzle_names[] = {
		"RGBA",
		"R", "G", "B", "A",
		"RGB", "RRR", "GGG", "BBB", "AAA"
	};
	static const char* sampler_names[] = {
		"Linear",
		"Nearest"
	};
	ImGui::Combo("Swizzle", view_swizzle, swizzle_names, countof(swizzle_names));
	ImGui::SameLine();
	ImGui::Combo("Sampler", view_sampler, sampler_names, countof(sampler_names));
	ImGui::SameLine();
	ImGui::SliderInt("Level", view_level, 0, image->n_levels - 1);
	ImGui::SameLine();
	ImGui::SliderInt("Layer", view_layer, 0, image->n_layers - 1);
	ImGui::SameLine();
	ImGui::DragFloat("Zoom", view_zoom, 0.01f, 0.01f, 10.f);
	ImGui::SameLine();
	ImGui::DragFloatRange2("Range", view_range_min, view_range_max, 0.01f, 0.f, 1.f);
	ImGui::PopItemWidth();

	//ImGui::BeginChild("image", vec2(-2, -2), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::PushImageViewType(ImGui::ImageViewArguments{ (ImGui::ImageViewSwizzle)*view_swizzle, (ImGui::ImageViewSampler)*view_sampler, 
		(uint)*view_level, (uint)*view_layer, (ushort)packHalf1x16(*view_range_min), (ushort)packHalf1x16(*view_range_max) });
	auto sz = image->levels[*view_level].extent;
	ImGui::Image(image, vec2(sz) * (*view_zoom));
	ImGui::PopImageViewType();
	if (ImGui::IsItemHovered())
	{
		vec2 p0 = ImGui::GetItemRectMin();
		vec2 p1 = ImGui::GetItemRectMax();
		vec2 pos = ImGui::GetMousePos();
		auto uv = (pos - p0) / (p1 - p0);
		uvec2 pixel = (vec2)sz * uv;
		ImGui::BeginTooltip();
		ImGui::Text("Pixel: %s", str(pixel).c_str());
		ImGui::Text("UV: %s", str(uv).c_str());
		switch (image->format)
		{
		case graphics::Format_R32_SFLOAT:
		case graphics::Format_Depth16:
		case graphics::Format_Depth32:
			ImGui::Text("Value: %f", image->get_pixel(pixel.x, pixel.y, 0, 0).x);
			break;
		default:
			cvec4 color = image->get_pixel(pixel.x, pixel.y, 0, 0) * 255.f;
			ImGui::Text("Color: %s", str(color).c_str());
		}
		ImGui::EndTooltip();
	}
	//ImGui::EndChild();
}

void ModelPreviewer::init()
{
	if (!model)
	{
		layer = 1 << (int)app.renderer->render_tasks.size();

		model = Entity::create();
		model->layer = layer;
		model->add_component<cNode>();
		auto mesh = model->add_component<cMesh>();
		mesh->instance_id = 0;
		mesh->set_material_name(L"default");
	}
	if (!node)
	{
		node = Entity::create();
		node->add_component<cNode>();

		auto e_camera = Entity::create();
		{
			auto node = e_camera->add_component<cNode>();
			auto q = angleAxis(radians(-45.f), vec3(0.f, 1.f, 0.f));
			node->set_qut(angleAxis(radians(-45.f), q * vec3(1.f, 0.f, 0.f)) * q);
		}
		camera = e_camera->add_component<cCamera>();
		camera->layer = layer;
		node->add_child(e_camera);

		node->add_child(model);

		app.world->root->add_child(node);
	}
	if (!image)
	{
		image = graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(256, 256, 1), graphics::ImageUsageAttachment |
			graphics::ImageUsageTransferSrc | graphics::ImageUsageSampled);
	}
	if (!render_task)
	{
		render_task = app.renderer->add_render_task(RenderModeSimple, camera, { image->get_view() },
			graphics::ImageLayoutShaderReadOnly, false, false);
		render_task->mode = RenderModeWireframe;
	}

}

void ModelPreviewer::destroy()
{
	if (image)
	{
		auto _image = image;
		add_event([_image]() {
			graphics::Queue::get()->wait_idle();
			delete _image;
			return false;
		});
	}

	if (node)
	{
		auto _node = node;
		add_event([_node]() {
			graphics::Queue::get()->wait_idle();
			app.world->root->remove_child(_node);
			return false;
		});
		node = nullptr;
		model = nullptr;
		camera = nullptr;
	}

	if (render_task)
	{
		auto _render_task = render_task;
		add_event([_render_task]() {
			graphics::Queue::get()->wait_idle();
			app.renderer->remove_render_task(_render_task);
			return false;
		});
		render_task = nullptr;
	}

	layer = 1;
	zoom = 1.f;
	updated_frame = 0;
}

bool ModelPreviewer::update(uint changed_frame, bool show_image)
{
	if (changed_frame > updated_frame)
	{
		add_event([this]() {
			if (!model || !camera)
				return false;
			AABB bounds;
			vertex_count = 0;
			face_count = 0;
			model->forward_traversal([&](EntityPtr e) {
				if (auto node = e->get_component<cNode>(); node)
				{
					if (!node->bounds.invalid())
						bounds.expand(node->bounds);
				}
				if (auto mesh = e->get_component<cMesh>(); mesh)
				{
					if (mesh->mesh_res_id != -1)
					{
						auto& info = sRenderer::instance()->get_mesh_res_info(mesh->mesh_res_id);
						vertex_count += info.vtx_cnt;
						face_count += info.idx_cnt / 3;
					}
				}
			});
			auto camera_node = camera->node;
			if (!bounds.invalid())
			{
				auto pos = fit_camera_to_object(mat3(camera_node->g_qut), camera->fovy,
					camera->zNear, camera->aspect, bounds);
				auto q = angleAxis(radians(-45.f), vec3(0.f, 1.f, 0.f));
				camera_node->set_qut(angleAxis(radians(-45.f), q * vec3(1.f, 0.f, 0.f)) * q);
				camera_node->set_pos(pos);
				zoom = length(pos);
			}
			return false;
		}, 0.f, 2);

		updated_frame = changed_frame;
	}

	if (!show_image)
		return false;

	static bool wireframe = true;
	if (ImGui::Checkbox("Wireframe", &wireframe))
		render_task->mode = wireframe ? RenderModeWireframe : RenderModeSimple;
	ImGui::InvisibleButton("##image", vec2(image->extent));
	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	auto dl = ImGui::GetWindowDrawList();
	dl->AddImage(image, p0, p1);
	auto operating = ImGui::IsItemActive() || ImGui::IsItemHovered();
	if (operating)
	{
		auto camera_node = camera->node;

		auto get_tar = [&]() {
			return camera_node->global_pos() - camera_node->z_axis() * zoom;
		};

		auto& io = ImGui::GetIO();
		if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
		{
			disp /= vec2(image->extent);
			if (io.KeyAlt)
			{
				if (io.MouseDown[ImGuiMouseButton_Left])
				{
					disp *= -180.f;
					disp = radians(disp);
					auto qut = angleAxis(disp.x, vec3(0.f, 1.f, 0.f)) * camera_node->qut;
					qut = angleAxis(disp.y, qut * vec3(1.f, 0.f, 0.f)) * qut;
					camera_node->set_qut(qut);
					camera_node->set_pos(get_tar() + (qut * vec3(0.f, 0.f, 1.f)) * zoom);
				}
			}
		}
		if (auto scroll = io.MouseWheel; scroll != 0.f)
		{
			auto tar = get_tar();
			if (scroll < 0.f)
				zoom = zoom * 1.1f + 0.5f;
			else
				zoom = max(0.f, zoom / 1.1f - 0.5f);
			camera_node->set_pos(tar + camera_node->z_axis() * zoom);
		}
	}
	ImGui::Text("Vertex Count: %d, Face Count: %d", (int)vertex_count, (int)face_count);

	return operating;
}

void App::init()
{
	create("Editor", uvec2(800, 600), WindowFrame | WindowResizable | WindowMaximized, true, graphics_debug, graphics_configs);
	graphics::gui_set_clear(true, vec4(0.f));
	world->update_components = false;
	input->transfer_events = false;
	always_render = false;
	renderer->add_render_task(RenderModeCameraLight, nullptr, {}, graphics::ImageLayoutShaderReadOnly);

	auto root = world->root.get();
	root->add_component<cNode>();
	e_editor = Entity::create();
	e_editor->name = "[Editor]";
	e_editor->add_component<cNode>();
	e_editor->add_component<cCamera>();
	root->add_child(e_editor);

	for (auto w : windows)
		w->init();

	add_event([this]() {
		save_preferences();
		return true; 
	}, 30.f);
	main_window->native->destroy_listeners.add([this]() {
		save_preferences();
	}, "app"_h);
}

bool App::on_update()
{
	if (timeline_playing)
	{
		if (opened_timeline && e_timeline_host)
		{
			set_timeline_current_frame((int)timeline_current_frame + 1);
			render_frames++;
		}
		else
			timeline_playing = false;
	}
	return UniverseApplication::on_update();
}

static std::vector<std::function<bool()>> dialogs;

void App::on_gui()
{
	auto last_focused_scene = scene_window.first_view();

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Project"))
		{
			ImGui::OpenFileDialog("New Project", [this](bool ok, const std::filesystem::path& path) {
				if (ok)
					new_project(path);
			});
		}
		if (ImGui::MenuItem("Open Project"))
		{
			ImGui::OpenFileDialog("Open Project", [this](bool ok, const std::filesystem::path& path) {
				if (ok)
					open_project(path);
			});
		}
		if (ImGui::MenuItem("Close Project"))
			close_project();
		ImGui::Separator();
		if (ImGui::MenuItem("Save"))
			;
		if (ImGui::MenuItem("Save All"))
			;
		if (ImGui::MenuItem("Close"))
			;
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo (Ctrl+Z)"))
			cmd_undo();
		if (ImGui::MenuItem("Redo (Ctrl+Y)"))
			cmd_redo();
		if (ImGui::MenuItem(std::format("Clear Histories ({})", (int)histories.size()).c_str()))
		{
			history_idx = -1;
			histories.clear();
		}
		ImGui::Separator();
		show_entities_menu();
		ImGui::Separator();
		if (ImGui::MenuItem("Clear Selections"))
			selection.clear("app"_h);
		if (ImGui::MenuItem("Select Parent"))
			;
		if (ImGui::MenuItem("Select Children"))
			;
		if (ImGui::MenuItem("Invert Siblings"))
			;
		if (ImGui::MenuItem("Focus To Selected (F)"))
		{
			if (last_focused_scene)
				last_focused_scene->focus_to_selected();
		}
		if (ImGui::MenuItem("Selected To Focus (G)"))
		{
			if (last_focused_scene)
				last_focused_scene->selected_to_focus();
		}
		if (ImGui::BeginMenu("Camera"))
		{
			if (ImGui::MenuItem("Reset"))
			{
				if (last_focused_scene)
					last_focused_scene->reset_camera(""_h);
			}
			if (ImGui::MenuItem("X+"))
			{
				if (last_focused_scene)
					last_focused_scene->reset_camera("X+"_h);
			}
			if (ImGui::MenuItem("X-"))
			{
				if (last_focused_scene)
					last_focused_scene->reset_camera("X-"_h);
			}
			if (ImGui::MenuItem("Y+"))
			{
				if (last_focused_scene)
					last_focused_scene->reset_camera("Y+"_h);
			}
			if (ImGui::MenuItem("Y-"))
			{
				if (last_focused_scene)
					last_focused_scene->reset_camera("Y-"_h);
			}
			if (ImGui::MenuItem("Z+"))
			{
				if (last_focused_scene)
					last_focused_scene->reset_camera("Z+"_h);
			}
			if (ImGui::MenuItem("Z-"))
			{
				if (last_focused_scene)
					last_focused_scene->reset_camera("Z-"_h);
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();
		if (ImGui::BeginMenu("NavMesh"))
		{
			struct GenerateDialog : ImGui::Dialog
			{
				std::vector<EntityPtr> nodes;
				float agent_radius = 0.6f;
				float agent_height = 1.8f;
				float walkable_climb = 0.5f;
				float walkable_slope_angle = 45.f;

				static void open()
				{
					auto dialog = new GenerateDialog;
					dialog->title = "Generate Navmesh";
					Dialog::open(dialog);
				}

				void draw() override
				{
					bool open = true;
					if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_NoSavedSettings))
					{
						if (ImGui::TreeNode("Nodes"))
						{
							if (ImGui::Button("From Selection"))
							{
								auto entities = selection.get_entities();
								nodes = entities;
							}

							auto n = (int)nodes.size();
							auto size_changed = ImGui::InputInt("size", &n, 1, 1);
							ImGui::Separator();
							if (size_changed)
								nodes.resize(n);
							else
							{
								n = nodes.size();
								for (auto i = 0; i < n; i++)
								{
									ImGui::PushID(i);
									std::string name = nodes[i] ? nodes[i]->name : "[None]";
									ImGui::InputText(nodes[i] ? "" : "Drop Entity Here", &name, ImGuiInputTextFlags_ReadOnly);
									if (ImGui::BeginDragDropTarget())
									{
										if (auto payload = ImGui::AcceptDragDropPayload("Entity"); payload)
										{
											auto entity = *(EntityPtr*)payload->Data;
											nodes[i] = entity;
										}
									}
									ImGui::PopID();
								}
							}
							ImGui::TreePop();
						}
						ImGui::InputFloat("Agent Radius", &agent_radius);
						ImGui::InputFloat("Agent Height", &agent_height);
						ImGui::InputFloat("Walkable Climb", &walkable_climb);
						ImGui::InputFloat("Walkable Slope Angle", &walkable_slope_angle);
						if (ImGui::Button("Generate"))
						{
							sScene::instance()->navmesh_generate(nodes, agent_radius, agent_height, walkable_climb, walkable_slope_angle);
							ImGui::CloseCurrentPopup();
							open = false;
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
						{
							ImGui::CloseCurrentPopup();
							open = false;
						}

						ImGui::End();
					}
					if (!open)
						close();
				}
			};

			if (ImGui::MenuItem("Generate"))
			{
				if (e_prefab)
					GenerateDialog::open();
			}

			if (ImGui::MenuItem("Save"))
			{
				ImGui::OpenFileDialog("Save Navmesh", [](bool ok, const std::filesystem::path& path) {
					if (ok)
						sScene::instance()->navmesh_save(path);
				}, Path::get(L"assets"));
			}

			if (ImGui::MenuItem("Load"))
			{
				ImGui::OpenFileDialog("Load Navmesh", [](bool ok, const std::filesystem::path& path) {
					if (ok)
						sScene::instance()->navmesh_load(path);
				}, Path::get(L"assets"));
			}

			if (ImGui::MenuItem("Export Model"))
			{
				struct ExportDialog : ImGui::Dialog
				{
					std::filesystem::path filename;
					bool merge_vertices = false;
					bool calculate_normals = false;

					static void open()
					{
						auto dialog = new ExportDialog;
						dialog->title = "Navmesh Export Model";
						Dialog::open(dialog);
					}

					void draw() override
					{
						bool open = true;
						if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_NoSavedSettings))
						{
							auto s = filename.string();
							ImGui::InputText("File Name", s.data(), ImGuiInputTextFlags_ReadOnly);
							ImGui::SameLine();
							if (ImGui::Button("..."))
							{
								ImGui::OpenFileDialog("File Name", [this](bool ok, const std::filesystem::path& path) {
									if (ok)
										filename = path;
								}, Path::get(L"assets"));
							}
							ImGui::Checkbox("Merge Vertices", &merge_vertices);
							ImGui::Checkbox("Calculate Normals", &calculate_normals);
							if (ImGui::Button("Export"))
							{
								auto points = sScene::instance()->navmesh_get_mesh();
								if (!points.empty())
								{
									std::vector<uint> indices;
									std::vector<vec3> normals;
									if (merge_vertices)
									{
										// TODO: fix bugs
										//struct Vec3Hasher
										//{
										//	bool operator()(const vec3& a, const vec3& b) const
										//	{
										//		return (std::hash<float>{}(a[0]) ^ std::hash<float>{}(a[1]) ^ std::hash<float>{}(a[2])) <
										//			(std::hash<float>{}(b[0]) ^ std::hash<float>{}(b[1]) ^ std::hash<float>{}(b[2]));
										//	}
										//};

										//std::map<vec3, uint, Vec3Hasher> map;
										//for (auto& p : points)
										//{
										//	auto it = map.find(p);
										//	if (it == map.end())
										//	{
										//		auto index = (uint)map.size();
										//		map[p] = index;
										//		indices.push_back(index);
										//	}
										//	else
										//		indices.push_back(it->second);
										//}
										//points.clear();
										//for (auto& [p, i] : map)
										//	points.push_back(p);
									}
									else
									{
										for (auto i = 0; i < points.size(); i++)
											indices.push_back(i);
									}
									if (calculate_normals)
									{
										normals.resize(points.size());
										for (auto i = 0; i < indices.size(); i += 3)
										{
											auto& a = points[indices[i]];
											auto& b = points[indices[i + 1]];
											auto& c = points[indices[i + 2]];
											auto n = normalize(cross(b - a, c - a));
											normals[indices[i]] += n;
											normals[indices[i + 1]] += n;
											normals[indices[i + 2]] += n;
										}
										for (auto& n : normals)
											n = normalize(n);
									}

									auto mesh = graphics::Mesh::create();
									mesh->positions = std::move(points);
									mesh->indices = std::move(indices);
									mesh->normals = std::move(normals);
									mesh->save(filename);
									delete mesh;
								}
								ImGui::CloseCurrentPopup();
								open = false;
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel"))
							{
								ImGui::CloseCurrentPopup();
								open = false;
							}

							ImGui::End();
						}

						if (!open)
							close();
					}
				};

				ExportDialog::open();
			}

			if (ImGui::MenuItem("Test"))
			{
				struct TestDialog : ImGui::Dialog
				{
					vec3 start = vec3(0.f);
					vec3 end = vec3(0.f);
					std::vector<vec3> points;

					static void open()
					{
						auto dialog = new TestDialog;
						dialog->title = "Navmesh Test";
						Dialog::open(dialog);
					}

					void draw() override
					{
						bool open = true;
						if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_NoSavedSettings))
						{
							static int v = 0;
							ImGui::TextUnformatted("use ctrl+click to set start/end");
							ImGui::RadioButton("Start", &v, 0);
							ImGui::TextUnformatted(("    " + str(start)).c_str());
							ImGui::RadioButton("End", &v, 1);
							ImGui::TextUnformatted(("    " + str(end)).c_str());
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl))
							{
								if (auto fv = scene_window.first_view(); fv)
								{
									if (v == 0)
										start = fv->hovering_pos;
									else
										end = fv->hovering_pos;
								}
								if (distance(start, end) > 0.f)
									points = sScene::instance()->navmesh_query_path(start, end);
							}


							{
								std::vector<vec3> points;
								points.push_back(start - vec3(1, 0, 0));
								points.push_back(start + vec3(1, 0, 0));
								points.push_back(start - vec3(0, 0, 1));
								points.push_back(start + vec3(0, 0, 1));
								sRenderer::instance()->draw_primitives(PrimitiveLineList, points.data(), points.size(), cvec4(0, 255, 0, 255));
							}
							{
								std::vector<vec3> points;
								points.push_back(end - vec3(1, 0, 0));
								points.push_back(end + vec3(1, 0, 0));
								points.push_back(end - vec3(0, 0, 1));
								points.push_back(end + vec3(0, 0, 1));
								sRenderer::instance()->draw_primitives(PrimitiveLineList, points.data(), points.size(), cvec4(0, 0, 255, 255));
							}
							if (!points.empty())
								sRenderer::instance()->draw_primitives(PrimitiveLineList, points.data(), points.size(), cvec4(255, 0, 0, 255));

							ImGui::End();
						}
						if (!open)
							close();
					}
				};

				if (e_prefab)
					TestDialog::open();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Blueprint"))
		{
			if (ImGui::MenuItem("Unify Variable Nodes"))
			{
				for (auto& it : std::filesystem::recursive_directory_iterator(Path::get(L"assets")))
				{
					if (it.is_regular_file() && it.path().extension() == L".bp")
					{
						auto changed = false;
						auto bp = Blueprint::get(it.path());
						for (auto& g : bp->groups)
						{
							union Key
							{
								uint64 u64;
								uint32 u32[2];
							};
							std::map<uint64, std::vector<BlueprintNodePtr>> var_nodes;

							for (auto& n : g->nodes)
							{
								if (n->name_hash == "Variable"_h)
								{
									Key key;
									uint var_name, var_location;
									key.u32[0] = *(uint*)n->inputs[0]->data;
									key.u32[1] = *(uint*)n->inputs[1]->data;
									var_nodes[key.u64].push_back(n.get());
								}
							}

							auto y = 0.f;
							for (auto& kv : var_nodes)
							{
								Key key;
								key.u64 = kv.first;

								if (kv.second.front()->depth > 1)
								{
									// insert a new top node
									auto new_n = bp->add_variable_node(g.get(), g->nodes.front().get(), key.u32[0], "Variable"_h, key.u32[1]);
									kv.second.insert(kv.second.begin(), new_n);
									changed = true;
								}
								if (kv.second.front()->position != vec2(0.f, y))
								{
									kv.second.front()->position = vec2(0.f, y);
									changed = true;
								}
								if (kv.second.size() > 1)
								{
									auto from_slot = kv.second.front()->outputs[0].get();

									for (auto i = 1; i < kv.second.size(); i++)
									{
										std::vector<BlueprintSlotPtr> staging_outputs;
										auto n = kv.second[i];
										auto output = n->outputs[0].get();
										staging_outputs.resize(output->get_linked_count());
										for (auto j = 0; j < staging_outputs.size(); j++)
											staging_outputs[j] = output->get_linked(j);
										bp->remove_node(n);
										for (auto& s : staging_outputs)
											bp->add_link(from_slot, s);
									}
									changed = true;
								}
								y += 100.f;
							}
						}
						if (changed)
						{
							auto is_editing = false;
							for (auto& v : blueprint_window.views)
							{
								if (auto bv = (BlueprintView*)v.get(); bv->blueprint == bp)
								{
									if (bv->unsaved)
										is_editing = true;
									break;
								}
							}
							if (!is_editing)
								bp->save();
						}
						Blueprint::release(bp);
					}
				}
			}
			if (ImGui::MenuItem("Refactor"))
			{
				enum class RefactorBlueprints
				{
					AllBlueprints,
					InFolders,
					MatchNames
				};

				static const char* refactor_blueprints_names[] = {
					"All Blueprints",
					"In Folders",
					"Match Names"
				};

				struct BpGroupInvalids
				{
					std::string name;
					uint name_hash;
					uint nodes_count;
					uint links_count;
					std::vector<std::pair<BlueprintInvalidNode, std::string>> invalid_nodes;
					std::vector<std::pair<BlueprintInvalidInput, std::string>> invalid_inputs;
					std::vector<std::pair<BlueprintInvalidLink, std::string>> invalid_links;
				};

				struct BpInvalids
				{
					std::filesystem::path path;
					std::vector<BpGroupInvalids> group_invalids;
				};

				struct RefactorDialog
				{
					bool open = false;
					RefactorBlueprints refactor_blueprints = RefactorBlueprints::AllBlueprints;
					bool process_all_nodes = false;
					bool process_all_links = false;
					bool process_invalid_nodes = true;
					bool process_invalid_inputs = true;
					bool process_invalid_links = true;
					std::vector<std::filesystem::path> action_files;
					std::filesystem::path actions_folder;
					int selected_action = -1;

					std::vector<BpInvalids> invalids;
					bool only_show_unresolved = false;
					uint unresolved_nodes = 0;
					uint unresolved_inputs = 0;
					uint unresolved_links = 0;

					void refresh_action_files()
					{
						if (actions_folder.empty())
							return;
						action_files.clear();
						for (auto& it : std::filesystem::recursive_directory_iterator(actions_folder))
						{
							if (it.is_regular_file() && it.path().extension() == L".bp")
								action_files.push_back(it.path());
						}
					}

					void show_invalids()
					{
						ImGui::Checkbox("Only Show Unresolved", &only_show_unresolved);

						ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
						if (ImGui::TreeNode("Invalids"))
						{

							for (auto& inv : invalids)
							{
								auto bp_header_status = -1;
								auto open_bp_header = [&]() {
									bp_header_status = ImGui::TreeNode(inv.path.string().c_str());
								};

								for (auto& g : inv.group_invalids)
								{
									auto group_header_status = -1;
									auto open_group_header = [&]() {
										group_header_status = ImGui::TreeNode(std::format("{} (nodes: {}, links: {}) ", g.name, g.nodes_count, g.links_count).c_str());
									};

									for (auto& n : g.invalid_nodes)
									{

									}
									if (bp_header_status == 0)
										break;
									if (group_header_status == 0)
										continue;

									for (auto& i : g.invalid_inputs)
									{
										if (only_show_unresolved && !i.second.empty())
											continue;
										if (bp_header_status == -1)
										{
											open_bp_header();
											if (bp_header_status == 0)
												break;
										}
										if (group_header_status == -1)
										{
											open_group_header();
											if (group_header_status == 0)
												break;
										}
										auto title = std::format("Node({})'s {}={}", i.first.node, i.first.name, i.first.value);
										if (i.second.empty())
											ImGui::Text(title.c_str());
										else
										{
											if (ImGui::TreeNode(title.c_str()))
											{
												ImGui::Text(i.second.c_str());
												ImGui::TreePop();
											}
										}
									}
									if (bp_header_status == 0)
										break;
									if (group_header_status == 0)
										continue;

									for (auto& l : g.invalid_links)
									{
										if (only_show_unresolved && !l.second.empty())
											continue;
										if (bp_header_status == -1)
										{
											open_bp_header();
											if (bp_header_status == 0)
												break;
										}
										if (group_header_status == -1)
										{
											open_group_header();
											if (group_header_status == 0)
												break;
										}
										auto title = std::format("From: {} {}({}), To: {} {}({})", 
											(l.first.reason & BlueprintInvalidFromNode) ? -(int)l.first.from_node : (int)l.first.from_node, 
											l.first.from_slot_name, l.first.from_slot,
											(l.first.reason & BlueprintInvalidFromNode) ? -(int)l.first.to_node : (int)l.first.to_node,
											l.first.to_slot_name, l.first.to_slot);
										if (l.second.empty())
											ImGui::Text(title.c_str());
										else
										{
											if (ImGui::TreeNode(title.c_str()))
											{
												ImGui::Text(l.second.c_str());
												ImGui::TreePop();
											}
										}
									}
									if (bp_header_status == 0)
										break;
									if (group_header_status == 0)
										continue;

									if (group_header_status == 1)
										ImGui::TreePop();
								}
								if (bp_header_status == 1)
									ImGui::TreePop();
							}

							ImGui::TreePop();
						}
					}

					bool refactor(bool is_preview)
					{
						if (selected_action == -1)
						{
							ImGui::OpenMessageDialog("Failed to refactor", "No action selected");
							return false;
						}
						auto path = Path::get(action_files[selected_action]);
						if (!std::filesystem::exists(path))
						{
							ImGui::OpenMessageDialog("Failed to refactor", "Action file not found");
							return false;
						}
						if (path.extension() != L".bp")
						{
							assert(0);
							return false;
						}
						auto action_bp = Blueprint::get(path);
						if (!action_bp)
						{
							ImGui::OpenMessageDialog("Failed to refactor", "Action is not a valid bp");
							return false;
						}

						auto ok = true;
						auto action_ins = BlueprintInstance::create(action_bp);
						if (auto action = action_ins->find_group("main"_h); action)
						{
							set_blueprint_refactoring_environment(is_preview, nullptr);

							if (process_all_nodes || process_all_links)
							{

							}
							if (process_invalid_nodes || process_invalid_inputs || process_invalid_links)
							{
								for (auto& inv : invalids)
								{
									auto bp = Blueprint::create(true);
									bp->load(inv.path);

									auto changed = false;
									for (auto& gi : inv.group_invalids)
									{
										if (auto g = bp->find_group(gi.name_hash); g)
										{
											action->set_variable_as("group"_h, g);
											if (process_invalid_nodes)
											{

											}
											if (process_invalid_inputs)
											{
												for (auto& i : gi.invalid_inputs)
												{
													if (is_preview)
													{
														i.second.clear();
														set_blueprint_refactoring_environment(true, &i.second);
													}
													else
													{
														if (i.second.empty())
															continue;
														changed = true;
													}
													action->reset_all_variables();
													action->set_variable_as("invalid_input_node"_h, g->find_node_by_id(i.first.node));
													action->set_variable_as("invalid_input_name"_h, i.first.name);
													action->set_variable_as("invalid_input_value"_h, i.first.value);
													action_ins->call(action, nullptr, nullptr);
												}
											}
											if (process_invalid_links)
											{
												for (auto& l : gi.invalid_links)
												{
													if (is_preview)
													{
														l.second.clear();
														set_blueprint_refactoring_environment(true, &l.second);
													}
													else
													{
														if (l.second.empty())
															continue;
														changed = true;
													}
													auto from_node = g->find_node_by_id(l.first.from_node);
													auto to_node = g->find_node_by_id(l.first.to_node);
													action->reset_all_variables();
													action->set_variable_as("invalid_link_from_node"_h, from_node);
													action->set_variable_as("invalid_link_from_slot"_h, from_node ? from_node->find_output(l.first.from_slot) : nullptr);
													action->set_variable_as("invalid_link_from_slot_name"_h, l.first.from_slot_name);
													action->set_variable_as("invalid_link_to_node"_h, to_node);
													action->set_variable_as("invalid_link_to_slot"_h, to_node ? to_node->find_input(l.first.to_slot) : nullptr);
													action->set_variable_as("invalid_link_to_slot_name"_h, l.first.to_slot_name);
													action_ins->call(action, nullptr, nullptr);
												}
											}
										}
									}

									if (!is_preview)
									{
										if (changed)
										{
											auto is_editing = false;
											for (auto& v : blueprint_window.views)
											{
												if (auto bv = (BlueprintView*)v.get(); bv->blueprint == bp)
												{
													if (bv->unsaved)
														is_editing = true;
													break;
												}
											}
											if (!is_editing)
												bp->save();
										}
									}

									Blueprint::destroy(bp);
								}
							}
						}
						else
							assert(0);

						BlueprintInstance::destroy(action_ins);
						Blueprint::release(action_bp);

						return ok;
					}
				};
				static RefactorDialog refactor_dialog;

				dialogs.push_back([&]() {
					if (!refactor_dialog.open)
					{
						refactor_dialog.open = true;
						if (auto flame_path = getenv("FLAME_PATH"); flame_path)
						{
							refactor_dialog.actions_folder = std::filesystem::path(flame_path);
							refactor_dialog.actions_folder /= L"tools/editor/refactorers";
							if (!std::filesystem::exists(refactor_dialog.actions_folder))
								std::filesystem::create_directories(refactor_dialog.actions_folder);
						}
						refactor_dialog.refresh_action_files();
					}

					if (ImGui::Begin("Refactor"))
					{
						if (ImGui::BeginCombo("Refactor Blueprints", refactor_blueprints_names[(int)refactor_dialog.refactor_blueprints]))
						{
							for (auto i = 0; i < countof(refactor_blueprints_names); i++)
							{
								if (ImGui::Selectable(refactor_blueprints_names[i], refactor_dialog.refactor_blueprints == (RefactorBlueprints)i))
									refactor_dialog.refactor_blueprints = (RefactorBlueprints)i;
							}
							ImGui::EndCombo();
						}
						ImGui::Checkbox("Process All Nodes", &refactor_dialog.process_all_nodes);
						ImGui::Checkbox("Process All Links", &refactor_dialog.process_all_links);
						ImGui::Checkbox("Process Invalid Nodes", &refactor_dialog.process_invalid_nodes);
						ImGui::Checkbox("Process Invalid Inputs", &refactor_dialog.process_invalid_inputs);
						ImGui::Checkbox("Process Invalid Links", &refactor_dialog.process_invalid_links);

						if (ImGui::Button("Collect Invalids"))
						{
							refactor_dialog.invalids.clear();

							auto assets_path = app.project_path / L"assets";
							for (auto& it : std::filesystem::recursive_directory_iterator(assets_path))
							{
								if (it.is_regular_file() && it.path().extension() == L".bp")
								{
									auto bp = Blueprint::create(true);
									bp->load(it.path());

									BpInvalids* bp_invalids = nullptr;
									for (auto& g : bp->groups)
									{
										if (!g->invalid_nodes.empty() || !g->invalid_inputs.empty() || !g->invalid_links.empty())
										{
											if (!bp_invalids)
											{
												auto& invalids = refactor_dialog.invalids.emplace_back();
												invalids.path = Path::reverse(it.path());
												bp_invalids = &invalids;
											}
											auto& invalids = bp_invalids->group_invalids.emplace_back();
											invalids.name = g->name;
											invalids.name_hash = g->name_hash;
											invalids.nodes_count = g->nodes.size();
											invalids.links_count = g->links.size();
											invalids.invalid_nodes.resize(g->invalid_nodes.size());
											for (auto i = 0; i < g->invalid_nodes.size(); i++)
												invalids.invalid_nodes[i].first = g->invalid_nodes[i];
											invalids.invalid_inputs.resize(g->invalid_inputs.size());
											for (auto i = 0; i < g->invalid_inputs.size(); i++)
												invalids.invalid_inputs[i].first = g->invalid_inputs[i];
											invalids.invalid_links.resize(g->invalid_links.size());
											for (auto i = 0; i < g->invalid_links.size(); i++)
												invalids.invalid_links[i].first = g->invalid_links[i];
										}
									}
									Blueprint::destroy(bp);
								}
							}
						}

						refactor_dialog.show_invalids();

						ImGui::Separator();

						if (ImGui::BeginListBox("Actions"))
						{
							for (auto i = 0; i < refactor_dialog.action_files.size(); i++)
							{
								if (ImGui::Selectable(refactor_dialog.action_files[i].filename().string().c_str(), refactor_dialog.selected_action == i))
									refactor_dialog.selected_action = i;
							}
							ImGui::EndListBox();
						}
						refactor_dialog.selected_action = min(refactor_dialog.selected_action, (int)refactor_dialog.action_files.size() - 1);

						if (ImGui::SmallButton(graphics::font_icon_str("plus"_h).c_str()))
						{
							ImGui::OpenInputDialog("New Action", "Name", [](bool ok, const std::string& str) {
								if (ok && !str.empty())
								{
									if (refactor_dialog.actions_folder.empty())
										return;
									auto fn = refactor_dialog.actions_folder / str;
									fn.replace_extension(L".bp");
									if (!std::filesystem::exists(fn))
									{
										auto bp = Blueprint::create();
										auto g = bp->groups.front().get();
										bp->add_variable(g, "group", TypeInfo::get<BlueprintGroupPtr>());
										bp->add_variable(g, "invalid_input_node", TypeInfo::get<BlueprintNodePtr>());
										bp->add_variable(g, "invalid_input_name", TypeInfo::get<std::string>());
										bp->add_variable(g, "invalid_input_value", TypeInfo::get<std::string>());
										bp->add_variable(g, "invalid_link_from_node", TypeInfo::get<BlueprintNodePtr>());
										bp->add_variable(g, "invalid_link_from_slot", TypeInfo::get<BlueprintSlotPtr>());
										bp->add_variable(g, "invalid_link_from_slot_name", TypeInfo::get<std::string>());
										bp->add_variable(g, "invalid_link_to_node", TypeInfo::get<BlueprintNodePtr>());
										bp->add_variable(g, "invalid_link_to_slot", TypeInfo::get<BlueprintSlotPtr>());
										bp->add_variable(g, "invalid_link_to_slot_name", TypeInfo::get<std::string>());
										bp->save(fn);

										refactor_dialog.refresh_action_files();
									}
									else
										ImGui::OpenMessageDialog("Failed to create action", "Action already existed");
								}
							});
						}
						ImGui::SameLine();
						if (ImGui::SmallButton(graphics::font_icon_str("minus"_h).c_str()))
						{
							if (refactor_dialog.selected_action != -1)
							{
								auto path = Path::get(refactor_dialog.action_files[refactor_dialog.selected_action]);
								std::filesystem::remove(path);
								refactor_dialog.refresh_action_files();
							}
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Edit"))
						{
							if (refactor_dialog.selected_action != -1)
							{
								auto path = Path::get(refactor_dialog.action_files[refactor_dialog.selected_action]);
								auto opened = false;
								for (auto& v : blueprint_window.views)
								{
									auto bv = (BlueprintView*)v.get();
									if (bv->blueprint_path == path)
									{
										if (bv->imgui_window)
											ImGui::FocusWindow((ImGuiWindow*)bv->imgui_window);
										opened = true;
										break;
									}
								}
								if (!opened)
									blueprint_window.open_view(Path::reverse(path).string() + "##Blueprint");
							}
						}

						ImGui::Separator();

						if (ImGui::Button("Refactor"))
						{
							if (refactor_dialog.refactor(true))
							{
								refactor_dialog.unresolved_nodes = 0;
								refactor_dialog.unresolved_inputs = 0;
								refactor_dialog.unresolved_links = 0;

								for (auto& inv : refactor_dialog.invalids)
								{
									for (auto& g : inv.group_invalids)
									{
										for (auto& n : g.invalid_nodes)
										{
											if (n.second.empty())
												refactor_dialog.unresolved_nodes++;
										}
										for (auto& i : g.invalid_inputs)
										{
											if (i.second.empty())
												refactor_dialog.unresolved_inputs++;
										}
										for (auto& l : g.invalid_links)
										{
											if (l.second.empty())
												refactor_dialog.unresolved_links++;
										}
									}
								}

								ImGui::OpenPopup("Result Preview");
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
							refactor_dialog.open = false;

						if (ImGui::BeginPopupModal("Result Preview"))
						{
							ImGui::Text("Unresolved Nodes: %d, Unresolved Inputs: %d, Unresolved Links: %d", 
								refactor_dialog.unresolved_nodes,
								refactor_dialog.unresolved_inputs,
								refactor_dialog.unresolved_links);

							refactor_dialog.show_invalids();

							if (ImGui::Button("Comfire"))
							{
								refactor_dialog.refactor(false);
								refactor_dialog.invalids.clear();
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel"))
								ImGui::CloseCurrentPopup();

							ImGui::EndPopup();
						}

						ImGui::End();
					}

					return refactor_dialog.open;
				});
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Preferences"))
		{
			struct PreferencesDialog
			{
				bool open = false;
			};
			static PreferencesDialog preferences_dialog;
			dialogs.push_back([&]() {
				if (!preferences_dialog.open)
				{
					preferences_dialog.open = true;
					ImGui::OpenPopup("Preferences");
				}

				if (ImGui::BeginPopupModal("Preferences", nullptr, ImGuiWindowFlags_NoSavedSettings))
				{
					ImGui::Checkbox("Use Flame Debugger", &preferences.use_flame_debugger);
					if (ImGui::Button("Save"))
					{
						save_preferences();
						preferences_dialog.open = false;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Button("Close"))
					{
						preferences_dialog.open = false;
						ImGui::CloseCurrentPopup();
					}
					ImGui::End();
				}
				return preferences_dialog.open;
			});
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Project"))
	{
		if (ImGui::MenuItem("Open In VS"))
		{
			auto vs_path = get_special_path("Visual Studio Installation Location");
			auto devenv_path = vs_path / L"Common7\\IDE\\devenv.exe";
			auto build_path = project_path / L"build";
			if (std::filesystem::exists(build_path))
			{
				auto sln_path = glob_files(build_path, L".sln")[0];
				exec(devenv_path, std::format(L"\"{}\"", sln_path.wstring()));
			}
		}
		if (ImGui::MenuItem("Attach Debugger"))
			vs_automate({ L"attach_debugger" });
		if (ImGui::MenuItem("Detach Debugger"))
			vs_automate({ L"detach_debugger" });
		if (ImGui::MenuItem("Do CMake"))
			cmake_project();
		if (ImGui::MenuItem("Build (Ctrl+B)"))
			build_project();
		if (ImGui::MenuItem("Package Game"))
			package_project();
		if (ImGui::MenuItem("Clean"))
		{
			if (!project_path.empty())
			{
				auto cpp_path = project_path / L"bin/debug/cpp.dll";
				cpp_path.replace_extension(L".pdb");
				if (std::filesystem::exists(cpp_path))
					std::filesystem::remove(cpp_path);
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Window"))
	{
		for (auto w : windows)
		{
			auto opened = (bool)!w->views.empty();
			if (ImGui::MenuItem(w->name.c_str(), nullptr, opened))
				w->open_view(false);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Debug"))
	{
		if (ImGui::MenuItem("Calculate Hash"))
		{
			ImGui::OpenInputDialog("Calculate Hash", "string", [](bool ok, const std::string& str) {
				if (ok)
				{
					uint hash = sh(str.c_str());
					auto text = std::format("{}: {}", str, hash);
					ImGui::OpenMessageDialog("String Hash", text);
				}
			});
		}

		struct UIStatusDialog
		{
			bool open = false;

		};
		static UIStatusDialog ui_status_dialog;
		if (ImGui::MenuItem("UI Status", nullptr, &ui_status_dialog.open))
		{
			if (ui_status_dialog.open)
			{
				dialogs.push_back([&]() {
					if (ui_status_dialog.open)
					{
						ImGui::Begin("UI Status", &ui_status_dialog.open, ImGuiWindowFlags_NoSavedSettings);
						ImGui::Text("Want Capture Mouse: %d", (int)graphics::gui_want_mouse());
						ImGui::Text("Want Capture Keyboard: %d", (int)graphics::gui_want_keyboard());
						ImGui::End();
					}
					return ui_status_dialog.open;
				});
			}
		}
		if (ImGui::MenuItem("Send Debug Cmd"))
		{
			ImGui::OpenInputDialog("Send Debug Cmd", "Cmd", [](bool ok, const std::string& str) {
				if (ok)
					sRenderer::instance()->send_debug_string(str);
			}, "", true);
		}
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PopStyleVar(2);

	// dock space
	ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0.0f, -20.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	// status bar
	ImGui::InvisibleButton("##status_bar", ImGui::GetContentRegionAvail());
	{
		auto dl = ImGui::GetWindowDrawList();
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		dl->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
		dl->AddText(p0, ImColor(255, 255, 255, 255), !last_status.empty() ? last_status.c_str() : "OK");
	}
	ImGui::End();

	auto& io = ImGui::GetIO();
	if (!io.WantCaptureKeyboard)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)Keyboard_F5))
		{
			if (!e_playing)
				cmd_play();
			else
			{
				if (e_playing)
					cmd_stop();
			}
		}
		if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl) && ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey)Keyboard_B))
			build_project();
		if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl) && ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey)Keyboard_Z))
			cmd_undo();
		if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl) && ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey)Keyboard_Y))
			cmd_redo();
		if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey)Keyboard_GraveAccentAndTilde))
			ImGui::OpenPopup("document_select", ImGuiPopupFlags_NoOpenOverExistingPopup);
		for (auto w : windows)
		{
			for (auto& v : w->views)
				v->on_global_shortcuts();
		}
	}

	if (ImGui::BeginPopup("document_select", ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto closing = !ImGui::IsKeyDown((ImGuiKey)(ImGuiKey)Keyboard_GraveAccentAndTilde);

		ImGui::TextUnformatted("Sheets:");
		ImGui::Indent();
		std::vector<SheetView*> sheet_views;
		for (auto& v : sheet_window.views)
			sheet_views.push_back((SheetView*)v.get());
		std::sort(sheet_views.begin(), sheet_views.end(), [](const auto a, const auto b) { return a->name < b->name; });
		for (auto v : sheet_views)
		{
			ImGui::Selectable(v->sheet_path.string().c_str());
			if (closing && ImGui::IsItemHovered())
			{
				if (v->imgui_window)
					ImGui::FocusWindow((ImGuiWindow*)v->imgui_window);
			}
		}
		ImGui::Unindent();
		ImGui::TextUnformatted("Blueprints:");
		ImGui::Indent();
		std::vector<BlueprintView*> blueprint_views;
		for (auto& v : blueprint_window.views)
			blueprint_views.push_back((BlueprintView*)v.get());
		std::sort(blueprint_views.begin(), blueprint_views.end(), [](const auto a, const auto b) { return a->name < b->name; });
		for (auto& v : blueprint_views)
		{
			auto name = v->blueprint_path.string();
			if (blueprint_window.debugger->debugging &&
				blueprint_window.debugger->debugging->instance->blueprint == v->blueprint)
				name += " (debugging)";
			ImGui::Selectable(name.c_str());
			if (closing && ImGui::IsItemHovered())
			{
				if (v->imgui_window)
					ImGui::FocusWindow((ImGuiWindow*)v->imgui_window);
			}
		}
		ImGui::Unindent();

		if (closing)
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	for (auto it = dialogs.begin(); it != dialogs.end();)
	{
		if (!(*it)())
			it = dialogs.erase(it);
		else
			it++;
	}
}

void App::load_preferences()
{
	auto preferences_i = parse_ini_file(preferences_path);
	for (auto& e : preferences_i.get_section_entries(""))
	{
		//if (e.key == "window_pos")
		//{
		//	auto pos = s2t<2, int>(e.values[0]);
		//	auto screen_size = get_screen_size();
		//	auto num_monitors = get_num_monitors();
		//	if (pos.x >= screen_size.x * num_monitors)
		//		pos.x = 0;
		//	main_window->native->set_pos(pos);
		//}
		if (e.key == "use_flame_debugger")
			preferences.use_flame_debugger = s2t<bool>(e.values[0]);
	}
	for (auto& e : preferences_i.get_section_entries("project_path"))
	{
		open_project(e.values[0]);
		break;
	}
	for (auto& e : preferences_i.get_section_entries("opened_views"))
	{
		for (auto w : windows)
		{
			auto name = e.values[0];
			if (name.ends_with(w->name))
			{
				w->open_view(name);
				break;
			}
		}
	}
	for (auto& e : preferences_i.get_section_entries("opened_folder"))
	{
		if (auto v = project_window.views.empty() ? nullptr : project_window.views.front().get(); v)
			((ProjectView*)v)->explorer.peeding_open_path = e.values[0];
		break;
	}
	for (auto& e : preferences_i.get_section_entries("opened_prefab"))
	{
		open_prefab(e.values[0]);
		break;
	}
	if (auto v = scene_window.first_view(); v)
	{
		v->edit_overlays.load(preferences_i.get_section_entries("scene_edit_overlays"));
		v->play_overlays.load(preferences_i.get_section_entries("scene_play_overlays"));
	}

	ImGui::GetIO().IniSavingRate = 10000.f;
}

void App::save_preferences()
{
	for (auto& p : project_settings.favorites)
		p = Path::reverse(p);
	project_settings.save();

	std::ofstream preferences_o(preferences_path);
	preferences_o << "window_pos=" + str(Application::main_window->pos) << "\n";
	preferences_o << "use_flame_debugger=" + str(preferences.use_flame_debugger) << "\n";
	if (!project_path.empty())
	{
		preferences_o << "[project_path]\n";
		preferences_o << project_path.string() << "\n";
	}
	preferences_o << "[opened_views]\n";
	for (auto w : windows)
	{
		for (auto& v : w->views)
			preferences_o << "\"" << v->get_save_name() << "\"\n";
	}
	if (auto fv = project_window.first_view(); fv && fv->explorer.opened_folder)
	{
		preferences_o << "[opened_folder]\n";
		preferences_o << fv->explorer.opened_folder->path.string() << "\n";
	}
	if (e_prefab)
	{
		preferences_o << "[opened_prefab]\n";
		preferences_o << prefab_path.string() << "\n";
	}
	if (auto v = scene_window.first_view(); v)
	{
		preferences_o << "[scene_edit_overlays]\n";
		v->edit_overlays.save(preferences_o);
		preferences_o << "[scene_play_overlays]\n";
		v->play_overlays.save(preferences_o);
	}
	preferences_o.close();

	ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
}

void App::new_project(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path))
	{
		wprintf(L"cannot create project: %s not exists\n", path.c_str());
		return;
	}
	if (!std::filesystem::is_empty(path))
	{
		auto n = 0;
		for (auto& it : std::filesystem::directory_iterator(path))
		{
			if (it.path().filename() != L".git")
				break;
			n++;
			if (n >= 2)
				break;
		}
		if (n == 0 || n >= 2)
		{
			wprintf(L"cannot create project: %s is not an empty directory\n", path.c_str());
			return;
		}
	}

	auto project_name = path.filename().string();

	auto assets_path = path / L"assets";
	std::filesystem::create_directories(assets_path);

	auto code_path = path / L"cpp";
	std::filesystem::create_directories(code_path);

	auto temp_path = path / L"temp";
	std::filesystem::create_directories(temp_path);

	open_project(path);
}

void App::open_project(const std::filesystem::path& path)
{
	if (!std::filesystem::is_directory(path))
		return;
	if (e_playing)
		return;

	close_project();

	project_path = path;
	directory_lock(project_path, true);

	auto assets_path = project_path / L"assets";
	if (std::filesystem::exists(assets_path))
	{
		Path::set_root(L"assets", assets_path);
		project_window.reset();
	}
	else
		assert(0);

	project_settings.load(project_path / L"project_settings.xml");
	for (auto& p : project_settings.favorites)
		p = Path::get(p);

	project_static_path = assets_path / L"static";
	project_static_path.make_preferred();
	if (std::filesystem::exists(project_static_path))
	{
		for (auto& it : std::filesystem::recursive_directory_iterator(project_static_path))
		{
			if (it.is_regular_file())
			{
				auto ext = it.path().extension();
				if (ext == L".sht")
				{
					auto sht = Sheet::get(it.path(), true);
					project_static_sheets.push_back(sht);
				}
				else if (ext == L".bp")
				{
					auto bp = Blueprint::get(it.path(), true);
					project_static_blueprints.push_back(bp);
				}
			}
		}
	}

	switch (project_settings.build_after_open)
	{
	case 0:
		load_project_cpp();
		break;
	case 1:
		build_project();
		break;
	case 2:
	{
		load_project_cpp();

		struct BuildProjectDialog
		{
			bool open = false;
		};
		static BuildProjectDialog build_project_dialog;
		add_event([]() {
			dialogs.push_back([&]() {
				if (!build_project_dialog.open)
				{
					build_project_dialog.open = true;
					ImGui::OpenPopup("Build Project");
				}

				if (ImGui::BeginPopupModal("Build Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
				{
					ImGui::Checkbox("Use Flame Debugger", &preferences.use_flame_debugger);
					if (ImGui::Button("OK"))
					{
						app.build_project();

						ImGui::CloseCurrentPopup();
						build_project_dialog.open = false;
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
						build_project_dialog.open = false;
					}
					ImGui::End();
				}

				return build_project_dialog.open;
			});
			return false;
		}, 0.f, 2);
	}
		break;
	}

	intellisense_collect();
}

void App::cmake_project()
{
	if (project_path.empty())
		return;
	if (e_playing)
	{
		printf("Cannot CMake project while playing\n");
		open_message_dialog("CMake Project", "Cannot CMake project while playing");
		return;
	}

	auto template_file = get_file_content(L"bp_project_cmake_template.txt");
	std::ofstream cmake_file(project_path / L"CMakeLists.txt");
	cmake_file << template_file;
	cmake_file.close();

	auto build_path = project_path;
	build_path /= L"build";
	shell_exec(L"cmake", std::format(L"-S \"{}\" -B \"{}\"", project_path.wstring(), build_path.wstring()), true);
}

void App::build_project()
{
	if (project_path.empty())
		return;
	if (e_playing)
	{
		printf("Cannot build project while playing\n");
		open_message_dialog("Build Project", "Cannot build project while playing");
		return;
	}

	if (/*is_cpp_project*/false)
	{
		auto old_prefab_path = prefab_path;
		if (!old_prefab_path.empty())
			close_prefab();

		add_event([this, old_prefab_path]() {
			unload_project_cpp();

			focus_window(get_console_window());

			cmake_project();

			auto cpp_project_path = project_path / L"build\\cpp.vcxproj";
			if (std::filesystem::exists(cpp_project_path))
			{
				vs_automate({ L"detach_debugger" });
				auto vs_path = get_special_path("Visual Studio Installation Location");
				auto msbuild_path = vs_path / L"Msbuild\\Current\\Bin\\MSBuild.exe";
				auto cwd = std::filesystem::current_path();
				std::filesystem::current_path(cpp_project_path.parent_path());
				printf("\n");
				auto cl = std::format(L"\"{}\" {}", msbuild_path.wstring(), cpp_project_path.filename().wstring());
				_wsystem(cl.c_str());
				std::filesystem::current_path(cwd);
				vs_automate({ L"attach_debugger" });
			}

			load_project_cpp();

			if (!old_prefab_path.empty())
				open_prefab(old_prefab_path);

			return false;
		});
	}
	else
	{
		cmake_project();

		auto template_file = get_file_content(L"bp_app_cpp_template.txt");
		std::ofstream cpp_file(project_path / L"app.cpp");
		cpp_file << template_file;
		cpp_file.close();

		auto vc_project_path = project_path / L"build\\PROJECT_NAME.vcxproj";
		if (std::filesystem::exists(vc_project_path))
		{
			auto vs_path = get_special_path("Visual Studio Installation Location");
			auto msbuild_path = vs_path / L"Msbuild\\Current\\Bin\\MSBuild.exe";
			auto cwd = std::filesystem::current_path();
			std::filesystem::current_path(vc_project_path.parent_path());
			printf("\n");
			auto cl = std::format(L"\"{}\" {}", msbuild_path.wstring(), vc_project_path.filename().wstring());
			_wsystem(cl.c_str());
			std::filesystem::current_path(cwd);
		}
	}
}

void App::package_project()
{
	if (project_path.empty())
		return;
	if (e_playing)
	{
		printf("Cannot package project while playing\n");
		open_message_dialog("Package Project", "Cannot package project while playing");
		return;
	}

	build_project();

	auto package_path = project_path / L"out";
	if (!std::filesystem::exists(package_path))
		std::filesystem::create_directories(package_path);
	std::filesystem::remove_all(package_path);
	std::filesystem::create_directories(project_path / L"out/flame");
	std::filesystem::create_directories(project_path / L"out/assets");

	for (auto& it : std::filesystem::directory_iterator(project_path / L"bin/debug"))
	{
		if (it.is_regular_file())
		{
			auto ext = it.path().extension();
			if (ext == L".dll" || ext == L".exe" || ext == L".pdb" || ext == L".typeinfo")
				std::filesystem::copy_file(it.path(), package_path / it.path().filename());
		}
	}
	if (auto flame_path = getenv("FLAME_PATH"); flame_path)
	{
		auto flame_assets_path = std::filesystem::path(flame_path) / L"assets";
		std::filesystem::copy(flame_assets_path, project_path / L"out/flame", std::filesystem::copy_options::recursive);
	}
	std::filesystem::copy(project_path / L"assets", project_path / L"out/assets", std::filesystem::copy_options::recursive);
}

void App::close_project()
{
	if (e_playing)
		return;

	close_prefab();

	if (!project_path.empty())
		directory_lock(project_path, false);
	project_path = L"";

	Path::set_root(L"assets", L"");
	project_window.reset();
	unload_project_cpp();
}

void App::new_prefab(const std::filesystem::path& path, uint type)
{
	auto e = Entity::create();
	switch (type)
	{
	case "general_3d_scene"_h:
		e->add_component<cNode>();
		auto e_camera = Entity::create();
		e_camera->name = "Camera";
		e_camera->add_component<cNode>();
		e_camera->add_component<cCamera>();
		e->add_child(e_camera);
		auto e_light = Entity::create();
		e_light->name = "Directional Light";
		e_light->add_component<cNode>()->set_eul(vec3(45.f, -60.f, 0.f));
		e_light->add_component<cDirectionalLight>();
		e->add_child(e_light);
		auto e_plane = Entity::create();
		e_plane->name = "Plane";
		e_plane->tag = e_plane->tag;
		e_plane->add_component<cNode>();
		e_plane->add_component<cMesh>()->set_mesh_and_material(L"standard_plane", L"default");
		e->add_child(e_plane);
		break;
	}
	e->save(path);
	delete e;
}

void App::open_prefab(const std::filesystem::path& path)
{
	if (e_playing || ev_open_prefab)
		return;
	close_prefab();
	prefab_path = path;
	prefab_unsaved = false;

	ev_open_prefab = add_event([this]() {
		e_prefab = Entity::create();
		e_prefab->load(prefab_path);
		world->root->add_child(e_prefab);
		ev_open_prefab = nullptr;
		return false;
	});
}

bool App::save_prefab()
{
	if (e_prefab && prefab_unsaved)
	{
		e_prefab->save(prefab_path);
		prefab_unsaved = false;
		last_status = std::format("Prefab Saved : {}", prefab_path.string());
	}
	return true;
}

void App::close_prefab()
{
	if (e_playing)
		return;
	prefab_path = L"";
	selection.clear("app"_h);

	if (e_prefab)
	{
		auto e = e_prefab;
		add_event([this, e]() {
			e->remove_from_parent();
			sScene::instance()->navmesh_clear();
			return false;
		});
		e_prefab = nullptr;
	}
}

void App::load_project_cpp()
{
	auto cpp_path = project_path / L"bin/debug/cpp.dll";
	if (std::filesystem::exists(cpp_path))
		project_cpp_library = tidb.load(cpp_path);
}

void App::unload_project_cpp()
{
	if (project_cpp_library)
	{
		tidb.unload(project_cpp_library);
		project_cpp_library = nullptr;
	}
}

void App::change_bp_references(uint old_name, uint old_location, uint old_property, uint new_name, uint new_location, uint new_property)
{
	auto assets_path = app.project_path / L"assets";
	for (auto& it : std::filesystem::recursive_directory_iterator(assets_path))
	{
		if (it.is_regular_file() && it.path().extension() == L".bp")
		{
			if (auto bp = Blueprint::get(it.path()); bp)
			{
				if (bp->name_hash != old_location)
				{
					if (bp->change_references(nullptr, old_name, old_location, old_property, new_name, new_location, new_property))
					{
						auto is_editing = false;
						for (auto& v : blueprint_window.views)
						{
							if (auto bv = (BlueprintView*)v.get(); bv->blueprint == bp)
							{
								if (bv->unsaved)
									is_editing = true;
								break;
							}
						}
						if (!is_editing)
							bp->save();
					}
				}
				Blueprint::release(bp);
			}
		}
	}
}

void App::open_timeline(const std::filesystem::path& path)
{
	close_timeline();

	opened_timeline = Timeline::load(path);
}

void App::close_timeline()
{
	if (opened_timeline)
	{
		delete opened_timeline;
		opened_timeline = nullptr;
		timeline_recording = false;
	}
}

void App::set_timeline_host(EntityPtr e)
{
	if (e_timeline_host)
	{
		e_timeline_host->message_listeners.remove("timeline_host"_h);
		e_timeline_host = nullptr;
	}
	if (e)
	{
		e->message_listeners.add([this](uint hash, void*, void*) {
			if (hash == "destroyed"_h)
			{
				e_timeline_host = nullptr;
				if (timeline_recording)
					timeline_recording = false;
			}
		}, "timeline_host"_h);
		e_timeline_host = e;
	}
	timeline_recording = false;
}

void App::set_timeline_current_frame(int frame)
{
	if (timeline_current_frame == frame || frame < 0)
		return;
	timeline_current_frame = frame;
	if (opened_timeline && e_timeline_host)
	{
		auto current_time = frame / 60.f;
		for (auto& t : opened_timeline->tracks)
		{
			float value;
			auto& keyframes = t.keyframes;
			if (keyframes.empty())
				continue;
			auto it = std::lower_bound(keyframes.begin(), keyframes.end(), current_time, [](const auto& a, auto t) {
				return a.time < t;
			});
			if (it == keyframes.end())
				value = s2t<float>(keyframes.back().value);
			else if (it == keyframes.begin())
				value = s2t<float>(keyframes.front().value);
			else
			{
				auto it2 = it - 1;
				auto t1 = it2->time;
				auto t2 = it->time;
				auto v1 = s2t<float>(it2->value);
				auto v2 = s2t<float>(it->value);
				value = mix(v1, v2, (current_time - t1) / (t2 - t1));
			}

			const Attribute* attr = nullptr; void* obj = nullptr; uint component_index;
			resolve_address(t.address, e_timeline_host, attr, obj, component_index);
			if (attr && attr->type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)attr->type;
				if (component_index < ti->vec_size)
				{
					auto pdata = attr->get_value(obj, true);
					switch (ti->data_type)
					{
					case DataFloat:
						((float*)pdata)[component_index] = value;
						break;
					}
					attr->set_value(obj, pdata);
				}
			}
		}
	}
}

void App::timeline_start_record()
{
	if (timeline_recording)
		return;
	if (e_timeline_host)
		timeline_recording = true;
}

void App::timeline_stop_record()
{
	if (!timeline_recording)
		return;
	timeline_recording = false;
}

KeyframePtr App::get_keyframe(const std::string& address, bool toggle)
{
	auto current_time = timeline_current_frame / 60.f;
	auto it = std::find_if(opened_timeline->tracks.begin(), opened_timeline->tracks.end(), [&](const auto& i) {
		return i.address == address;
	});
	if (it == opened_timeline->tracks.end())
	{
		auto& t = opened_timeline->tracks.emplace_back();
		t.address = address;
		return &t.keyframes.emplace_back(current_time, "");
	}

	auto& t = *it;
	auto it2 = std::find_if(t.keyframes.begin(), t.keyframes.end(), [&](const auto& i) {
		return i.time == current_time;
	});
	if (it2 == t.keyframes.end())
	{
		auto it3 = std::lower_bound(t.keyframes.begin(), t.keyframes.end(), current_time, [&](const auto& i, auto v) {
			return i.time < v;
		});
		return &*t.keyframes.emplace(it3, current_time, "");
	}

	if (toggle)
	{
		t.keyframes.erase(it2);
		if (t.keyframes.empty())
			opened_timeline->tracks.erase(it);
		return nullptr;
	}

	return &*it2;
}

void App::timeline_toggle_playing()
{
	if (timeline_playing)
	{
		timeline_playing = false;
		return;
	}

	if (opened_timeline && e_timeline_host)
		timeline_playing = true;
}

void App::open_file_in_vs(const std::filesystem::path& path)
{
	vs_automate({ L"open_file", path.wstring() });
}

void App::vs_automate(const std::vector<std::wstring>& cl)
{
	if (auto flame_path = getenv("FLAME_PATH"); flame_path)
	{
		auto automation_path = std::filesystem::path(flame_path) / L"bin/debug/vs_automation.exe";
		std::wstring cl_str;
		if (cl[0] == L"attach_debugger" || cl[0] == L"detach_debugger")
		{
			if (!preferences.use_flame_debugger)
				cl_str = L"-p " + project_path.filename().wstring();
			cl_str += L" -c " + cl[0];
			cl_str += L" " + wstr(getpid());
		}
		else if (cl[0] == L"open_file")
		{
			cl_str = L"-p " + project_path.filename().wstring();
			cl_str += L" -c open_file " + cl[1];
		}
		wprintf(L"vs automate: %s\n", cl_str.c_str());
		shell_exec(automation_path, cl_str, true);
	}
}

void App::intellisense_collect()
{
	intelli_sense.collect();
}

bool App::cmd_undo()
{
	if (history_idx < 0)
		return false;
	histories[history_idx]->undo();
	history_idx--;
	return true;
}

bool App::cmd_redo()
{
	if (history_idx + 1 >= histories.size())
		return false;
	history_idx++;
	histories[history_idx]->redo();
	return true;
}

bool App::cmd_new_entities(std::vector<EntityPtr>&& ts, uint type)
{
	if (ts.empty())
	{
		if (e_playing)
			ts.push_back(e_playing);
		else if (e_prefab)
			ts.push_back(e_prefab);
		else
			return false;
	}
	std::vector<EntityPtr> es;
	for (auto t : ts)
	{
		auto e = Entity::create();
		e->name = "entity";
		switch (type)
		{
		case "empty"_h:
			break;
		case "node"_h:
			e->add_component<cNode>();
			break;
		case "plane"_h:
			e->add_component<cNode>();
			e->add_component<cMesh>()->set_mesh_and_material(L"standard_plane", L"default");
			break;
		case "cube"_h:
			e->add_component<cNode>();
			e->add_component<cMesh>()->set_mesh_and_material(L"standard_cube", L"default");
			break;
		case "sphere"_h:
			e->add_component<cNode>();
			e->add_component<cMesh>()->set_mesh_and_material(L"standard_sphere", L"default");
			break;
		case "cylinder"_h:
			e->add_component<cNode>();
			e->add_component<cMesh>()->set_mesh_and_material(L"standard_cylinder", L"default");
			break;
		case "tri_prism"_h:
			e->add_component<cNode>();
			e->add_component<cMesh>()->set_mesh_and_material(L"standard_tri_prism", L"default");
			break;
		case "dir_light"_h:
			e->add_component<cNode>()->set_eul(vec3(45.f, -60.f, 0.f));
			e->add_component<cDirectionalLight>();
			break;
		case "pt_light"_h:
			e->add_component<cNode>();
			e->add_component<cPointLight>();
			break;
		case "camera"_h:
			e->add_component<cNode>();
			e->add_component<cCamera>();
			break;
		case "element"_h:
			e->add_component<cElement>();
			break;
		case "image"_h:
			e->add_component<cElement>();
			e->add_component<cImage>();
			break;
		case "text"_h:
			e->add_component<cElement>();
			e->add_component<cText>();
			break;
		case "layout"_h:
			e->add_component<cElement>();
			e->add_component<cLayout>();
			break;
		}
		t->add_child(e);
		es.push_back(e);

		if (auto ins = get_root_prefab_instance(t); ins)
			ins->mark_modification(e->parent->file_id.to_string() + (!e->prefab_instance ? '|' + e->file_id.to_string() : "") + "|add_child");
	}
	if (!e_playing && e_prefab)
	{
		std::vector<GUID> ids(es.size());
		std::vector<GUID> parents(es.size());
		std::vector<uint> indices(es.size());
		std::vector<EntityContent> contents(es.size());
		for (auto i = 0; i < es.size(); i++)
		{
			auto e = es[i];
			ids[i] = e->file_id;
			parents[i] = e->parent->instance_id;
			indices[i] = e->index;
			contents[i].init(e);
		}
		auto h = new EntityHistory(ids, {}, {}, parents, indices, contents);
		add_history(h);
		if (h->ids.size() == 1)
			app.last_status = std::format("Entity Created: {} (type: {})", es[0]->name, type);
		else
			app.last_status = std::format("{} Entities Created: (type: {})", (int)h->ids.size(), type);

		prefab_unsaved = true;
	}
	return true;
}

bool App::cmd_delete_entities(std::vector<EntityPtr>&& es)
{
	if (es.empty())
		return false;
	for (auto e : es)
	{
		if (e == e_prefab)
			return false;
		if (auto ins = get_root_prefab_instance(e); ins && ins != e->prefab_instance.get() &&
			ins->find_modification(e->parent->file_id.to_string() + (!e->prefab_instance ? '|' + e->file_id.to_string() : "") + "|add_child") == -1)
		{
			open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
			return false;
		}
	}
	
	std::vector<std::string> names;
	std::vector<GUID> ids;
	std::vector<GUID> parents;
	std::vector<uint> indices;
	std::vector<EntityContent> contents;
	if (!e_playing && e_prefab)
	{
		names.resize(es.size());
		ids.resize(es.size());
		parents.resize(es.size());
		indices.resize(es.size());
		contents.resize(es.size());
	}
	for (auto i = 0; i < es.size(); i++)
	{
		auto e = es[i];
		add_event([e]() {
			if (auto ins = get_root_prefab_instance(e); ins)
				ins->remove_modification(e->parent->file_id.to_string() + (!e->prefab_instance ? '|' + e->file_id.to_string() : "") + "|add_child");
			e->remove_from_parent();
			return false;
		});

		if (!e_playing && e_prefab)
		{
			names[i] = e->name;
			ids[i] = e->file_id;
			parents[i] = e->parent->instance_id;
			indices[i] = e->index;
			contents[i].init(e);
		}
	}

	auto is_selecting_entities = selection.type == Selection::tEntity;
	auto selected_entities = selection.get_entities();
	for (auto e : es)
	{
		if (is_selecting_entities)
		{
			if (auto it = std::find(selected_entities.begin(), selected_entities.end(), e); it != selected_entities.end())
				selected_entities.erase(it);
		}
	}
	if (is_selecting_entities)
		selection.select(selected_entities, "app"_h);

	if (!e_playing && e_prefab)
	{
		auto h = new EntityHistory(ids, {}, {}, parents, indices, contents);
		add_history(h);
		if (h->ids.size() == 1)
			app.last_status = std::format("Entity Removed: {}", names[0]);
		else
			app.last_status = std::format("{} Entities Removed", (int)h->ids.size());

		prefab_unsaved = true;
	}
	return true;
}

bool App::cmd_duplicate_entities(std::vector<EntityPtr>&& es)
{
	if (es.empty())
		return false;
	for (auto t : es)
	{
		if (t == e_prefab)
			return false;
	}
	std::vector<EntityPtr> new_entities;
	for (auto t : es)
	{
		auto new_one = t->duplicate();
		new_entities.push_back(new_one);
		t->parent->add_child(new_one);

		if (auto ins = get_root_prefab_instance(new_one->parent); ins)
			ins->mark_modification(new_one->parent->file_id.to_string() + (!new_one->prefab_instance ? '|' + new_one->file_id.to_string() : "") + "|add_child");
	}
	selection.select(new_entities, "app"_h);

	if (!e_playing && e_prefab)
	{
		std::vector<GUID> ids(new_entities.size());
		std::vector<GUID> parents(new_entities.size());
		std::vector<uint> indices(new_entities.size());
		std::vector<EntityContent> contents(new_entities.size());
		for (auto i = 0; i < new_entities.size(); i++)
		{
			auto e = new_entities[i];
			ids[i] = e->file_id;
			parents[i] = e->parent->instance_id;
			indices[i] = e->index;
			contents[i].init(e);
		}
		auto h = new EntityHistory(ids, {}, {}, parents, indices, contents);
		add_history(h);
		if (h->ids.size() == 1)
			app.last_status = std::format("Entity Dyplicated: {}", new_entities[0]->name);
		else
			app.last_status = std::format("{} Entities Dyplicated", (int)h->ids.size());

		prefab_unsaved = true;
	}
	return true;
}

bool App::cmd_play()
{
	if (!e_playing && e_prefab)
	{
		add_event([this]() {
			e_prefab->remove_from_parent(false);
			e_playing = e_prefab->duplicate();
			world->root->add_child(e_playing);
			world->update_components = true;
			input->transfer_events = true;
			always_render = true;
			paused = false;

			cCameraPtr camera = nullptr;
			e_playing->traversal_bfs([&](EntityPtr e, int) {
				if (!camera)
					camera = e->get_component<cCamera>();
				if (camera)
					return false;
				return true;
			});

			if (camera)
			{
				if (auto fv = scene_window.first_view(); fv)
				{
					auto& camera_list = cCamera::list();
					for (auto i = 0; i < camera_list.size(); i++)
					{
						if (camera_list[i] == camera)
							fv->camera_idx = i;
					}
				}
			}

			listeners.call("game_started"_h);

			return false;
		});

		return true;
	}
	else if (paused)
	{
		paused = false;
		world->update_components = true;
		input->transfer_events = true;
		return true;
	}

	return false;
}

bool App::cmd_pause()
{
	if (!e_playing || paused)
		return false;
	paused = true;
	world->update_components = false;
	input->transfer_events = false;
	return true;
}

bool App::cmd_stop()
{
	if (!e_playing)
		return false;

	add_event([this]() {
		sGraveyard::instance()->clear();
		sTween::instance()->clear();

		e_playing->remove_from_parent();
		e_playing = nullptr;
		world->root->add_child(e_prefab);
		world->update_components = false;
		always_render = false;

		auto fv = scene_window.first_view();
		auto& camera_list = cCamera::list();
		if (camera_list.size() > 0)
		{
			if (fv)
				fv->camera_idx = 0;
			sRenderer::instance()->render_tasks.front()->camera = camera_list.front();
		}
		else
		{
			if (fv)
				fv->camera_idx = -1;
			sRenderer::instance()->render_tasks.front()->camera = nullptr;
		}
		sRenderer::instance()->render_tasks.front()->mode = RenderModeCameraLight;

		listeners.call("game_stopped"_h);

		return false;
	});

	return true;
}

int entry(int argc, char** args)
{
	srand(time(0));

	auto ap = parse_args(argc, args);
	if (ap.has("-fixed_render_target_size"))
		scene_window.fixed_render_target_size = true;
	if (ap.has("-dont_use_mesh_shader"))
		app.graphics_configs.emplace_back("mesh_shader"_h, 0);
	if (ap.has("-replace_renderpass_attachment_dont_care_to_load"))
		app.graphics_configs.emplace_back("replace_renderpass_attachment_dont_care_to_load"_h, 1);
	if (ap.has("-image_additional_usage_transfer"))
		app.graphics_configs.emplace_back("image_additional_usage_transfer"_h, 1);

	app.init();
	app.load_preferences();
	app.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
