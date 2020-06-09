#pragma once

#include <flame/serialize.h>
#include <flame/graphics/image.h>

#include "../app.h"

struct _2DGizmo
{
	cElement* base;

	cElement* block_c;
	cElement* block_l;
	cElement* block_t;
	cElement* block_r;
	cElement* block_b;
	cElement* block_lt;
	cElement* block_rt;
	cElement* block_lb;
	cElement* block_rb;

	cElement* target;
	void* listener;

	_2DGizmo();
	void create();
	void on_select();
	void show_blocks(bool v);
	void update_blocks();
};

struct cSceneEditor : Component
{
	_2DEditor edt;

	int tool_type;

	_2DGizmo gizmo;

	cSceneEditor();
	~cSceneEditor() override;
	Entity* search_hovering(const Vec4f& r);
	void search_hovering_r(Entity* e, Entity*& s, const Vec4f& r);
	void on_select();
};

struct cResourceExplorer : Component
{
	std::filesystem::path base_path;
	std::filesystem::path curr_path;

	Entity* address_bar;
	Entity* e_list;
	cElement* c_list_element;
	cLayout* c_list_layout;
	Image* folder_img;
	uint folder_img_idx;
	Image* file_img;
	uint file_img_idx;
	Image* thumbnails_img;
	uint thumbnails_img_idx;
	Vec2u thumbnails_img_pos;
	std::vector<std::unique_ptr<Vec2u>> thumbnails_seats_free;
	std::vector<std::unique_ptr<Vec2u>> thumbnails_seats_occupied;

	std::filesystem::path selected;

	void* ev_file_changed;
	void* ev_end_file_watcher;

	cResourceExplorer();
	~cResourceExplorer() override;
	Entity* cResourceExplorer::create_listitem(const std::wstring& title, uint img_id);
	void cResourceExplorer::navigate(const std::filesystem::path& path);
	void cResourceExplorer::on_component_added(Component* c);
	void cResourceExplorer::draw(graphics::Canvas* canvas);
};

struct cHierarchy : Component
{
	Entity* e_tree;

	cHierarchy();
	~cHierarchy() override;
	Entity* find_item(Entity* e) const;
	void refresh_selected();
	void refresh();
};

struct cInspector : Component
{
	Entity* e_layout;

	cInspector();
	~cInspector() override;
	void refresh();
};

struct SceneEditorWindow : App::Form
{
	UI ui;

	SceneEditorWindow();
	~SceneEditorWindow();
};

struct SceneEditor
{
	SceneEditorWindow* window;

	std::filesystem::path filepath;
	Entity* prefab;
	bool changed;

	Entity* selected;

	cSceneEditor* editor;
	cResourceExplorer* resource_explorer;
	cHierarchy* hierarchy;
	cInspector* inspector;

	SceneEditor()
	{
		editor = nullptr;
		resource_explorer = nullptr;
		hierarchy = nullptr;
		inspector = nullptr;

		prefab = nullptr;
		selected = nullptr;
	}

	void select(Entity* e);
	void load(const std::filesystem::path& _filepath);
	void save();
};

extern SceneEditor scene_editor;
