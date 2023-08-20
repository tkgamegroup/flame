#pragma once

#include "app.h"

struct CommonComponents
{
	uint type_hash;
	std::vector<ComponentPtr> components;
};

struct InspectorView;

struct InspectedEntities
{
	InspectorView* inspector;
	std::vector<EntityPtr> entities;
	std::unordered_map<const void*, uint> sync_states;
	std::vector<CommonComponents> common_components;
	std::filesystem::path prefab_path;

	~InspectedEntities();
	void refresh(const std::vector<EntityPtr>& _entities);
	std::pair<uint, uint> manipulate();
};

struct InspectorView : View
{
	bool locked = false;
	Selection::Type inspected_type = Selection::tNothing;
	std::vector<std::filesystem::path> inspected_paths;
	void* inspected_obj = nullptr;
	void(*inspected_obj_deletor)(void* obj, void* info) = nullptr;
	char inspected_obj_info[1024];
	InspectedEntities inspected_entities;
	uint last_select_frame = 0;
	bool dirty = true;

	InspectorView();
	InspectorView(const std::string& name);
	~InspectorView();
	void on_draw() override;
};

struct InspectorWindow : Window
{
	InspectorWindow();
	void init() override;
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern InspectorWindow inspector_window;
