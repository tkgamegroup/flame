#pragma once

#include <flame/universe/application.h>
#include <flame/universe/project/project_settings.h>

using namespace flame;

inline std::string ti_str(TypeInfo* ti) 
{
	return TypeInfo::serialize_t(ti->tag) + '@' + ti->name;
}

struct DropPaths
{
	std::filesystem::path* paths;
	int count;
};

struct DropEnitities
{
	EntityPtr* entities;
	int count;
};

inline PrefabInstance* get_root_prefab_instance(EntityPtr e)
{
	PrefabInstance* ret = nullptr;
	while (e)
	{
		if (e->prefab_instance)
			ret = e->prefab_instance.get();
		e = e->parent;
	}
	return ret;
}

inline bool is_ancestor(EntityPtr t, EntityPtr e)
{
	if (!e->parent)
		return false;
	if (e->parent == t)
		return true;
	return is_ancestor(t, e->parent);
};

inline void empty_entity(EntityPtr e)
{
	e->name = "";
	e->tag = TagGeneral;
	e->enable = true;
	e->remove_all_components();
	e->remove_all_children();
}

inline std::string get_unique_name(const std::string& prefix, const std::function<bool(const std::string& name)>& check_name_exists)
{
	std::string name = prefix;
	int i = 0;
	while (check_name_exists(name))
	{
		name = prefix + str(i);
		i++;
	}
	return name;
}

struct View;

struct Window
{
	std::string name;
	std::vector<std::unique_ptr<View>> views;

	Window(const std::string& name);
	~Window();

	virtual void init() {}
	virtual View* open_view(bool new_instance) { return nullptr; }
	virtual View* open_view(const std::string& name) { return nullptr; }
};

extern std::vector<Window*> windows;

inline Window::Window(const std::string& name) :
	name(name)
{
	windows.push_back(this);
}

struct View
{
	Window* window;
	std::string name;
	void* imgui_window = nullptr;

	View(Window* w, const std::string& name) :
		window(w),
		name(name)
	{
		w->views.emplace_back(this);

		add_event([this]() {
			graphics::gui_callbacks.add([this]() {
				on_draw();
			}, (uint)this);
			return false;
		});
	}

	virtual ~View()
	{
		if (!window)
			return;

		for (auto it = window->views.begin(); it != window->views.end(); it++)
		{
			if (it->get() == this)
			{
				it->release();
				window->views.erase(it);
				break;
			}
		}

		add_event([this]() {
			graphics::gui_callbacks.remove((uint)this);
			return false;
		});
	}

	void title_context_menu();
	virtual void on_draw() = 0;
	virtual void on_global_shortcuts() {}
	virtual std::string get_save_name() { return name; }
};

inline Window::~Window()
{
	for (auto& v : views)
		v->window = nullptr;
}

bool filter_name(const std::string& name, const std::string& find_str, bool match_case, bool match_whole_word);
std::string get_value_str(TypeInfo* type, void* data);
void show_entities_menu();
TypeInfo* show_types_menu();
void open_message_dialog(const std::string& title, const std::string& message);
void view_image(graphics::ImagePtr image, int* view_swizzle, int* view_sampler, int* view_level, int* view_layer, float* view_zoom, float* view_range_min, float* view_range_max);

struct ModelPreviewer
{
	graphics::ImagePtr image = nullptr;
	uint layer = 1;
	EntityPtr node = nullptr;
	EntityPtr model = nullptr;
	cCameraPtr camera = nullptr;
	float zoom = 1.f;
	uint vertex_count = 0;
	uint face_count = 0;
	RenderTaskPtr render_task = nullptr;
	uint updated_frame = 0;

	void init();
	void destroy();
	bool update(uint changed_frame, bool show_image = true);
};

struct App : UniverseApplication
{
	bool graphics_debug = true;
	std::vector<std::pair<uint, uint>> graphics_configs;

	std::string last_status;

	std::filesystem::path project_path;
	std::filesystem::path project_static_path;
	std::filesystem::path prefab_path;
	bool prefab_unsaved = false;
	std::vector<SheetPtr> project_static_sheets;
	std::vector<BlueprintPtr> project_static_blueprints;
	void* project_cpp_library = nullptr;
	EntityPtr e_editor = nullptr;
	EntityPtr e_prefab = nullptr;
	EntityPtr e_playing = nullptr;
	EntityPtr e_preview = nullptr;
	bool paused = false;
	void* ev_open_prefab = nullptr;

	TimelinePtr opened_timeline = nullptr;
	EntityPtr e_timeline_host = nullptr;
	uint timeline_current_frame = 0;
	bool timeline_recording = false;
	bool timeline_playing = false;

	ProjectSettings project_settings;

	void init();
	bool on_update() override;
	void on_gui() override;

	void load_preferences();
	void save_preferences();
	void new_project(const std::filesystem::path& path);
	void open_project(const std::filesystem::path& path);
	void cmake_project();
	void build_project();
	void package_project();
	void close_project();
	void new_prefab(const std::filesystem::path& path, uint type = "empty"_h);
	void open_prefab(const std::filesystem::path& path);
	bool save_prefab();
	void close_prefab();
	void load_project_cpp();
	void unload_project_cpp();
	void change_bp_references(uint old_name, uint old_location, uint old_property, uint new_name, uint new_location, uint new_property);
	void open_timeline(const std::filesystem::path& path);
	void close_timeline();
	void set_timeline_host(EntityPtr e);
	void set_timeline_current_frame(int frame);
	void timeline_start_record();
	void timeline_stop_record();
	KeyframePtr get_keyframe(const std::string& address, bool toggle = false);
	void timeline_toggle_playing();
	void open_file_in_vs(const std::filesystem::path& path);
	void vs_automate(const std::vector<std::wstring>& cl);

	bool cmd_undo();
	bool cmd_redo();
	bool cmd_new_entities(std::vector<EntityPtr>&& es, uint type = "empty"_h);
	bool cmd_delete_entities(std::vector<EntityPtr>&& es);
	bool cmd_duplicate_entities(std::vector<EntityPtr>&& es);
	bool cmd_play();
	bool cmd_pause();
	bool cmd_stop();
	bool cmd_start_preview(EntityPtr e);
	bool cmd_stop_preview();
	bool cmd_restart_preview();
};

extern App app;
