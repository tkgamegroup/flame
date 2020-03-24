#pragma once

#include <flame/serialize.h>
#include <flame/universe/utils/ui.h>
#include <flame/utils/app.h>

using namespace flame;
using namespace graphics;

struct cEditor : Component
{
	cText* c_tab_text;
	Entity* e_base;
	cElement* c_base_element;
	bool base_moved;
	uint scale;
	cText* c_scale_text;

	bool selecting;
	Vec2f select_anchor_begin;
	Vec2f select_anchor_end;

	BP::Slot* dragging_slot;
	BP::Slot* pending_link_slot;
	Vec2f dragging_slot_pos;

	cEditor();
	virtual ~cEditor() override;
	void on_deselect();
	void on_select();
	void on_pos_changed(BP::Node* n);
	void on_changed();
	void on_add_node(BP::Node* n);
	void on_remove_node(BP::Node* n);
	void on_data_changed(BP::Slot* s);
	void base_scale(int v);
	void base_move(const Vec2f& p);
	void show_add_node_menu(const Vec2f& pos);
};

struct cConsole : Component
{
	cText* c_text_log;
	cEdit* c_edit_input;

	cConsole();
	virtual ~cConsole() override;
};

void undo();
void redo();

struct MyApp : App
{
	std::filesystem::path filepath;
	std::filesystem::path fileppath;
	BP* bp;
	bool changed;
	bool locked;
	bool auto_update;

	std::vector<BP::Node*> selected_nodes;
	std::vector<BP::Slot*> selected_links;

	cEditor* editor;
	cConsole* console;

	MyApp()
	{
		editor = nullptr;
		console = nullptr;

		bp = nullptr;
		changed = false;
		locked = false;

		auto_update = false;
	}

	void set_changed(bool v);
	void set_node_pos(BP::Node*n, const Vec2f& pos);

	void deselect();
	void select(const std::vector<BP::Node*>& nodes);
	void select(const std::vector<BP::Slot*>& links);

	BP::Library* add_library(const wchar_t* filename);
	BP::Node* add_node(const char* type_name, const char* id, const Vec2f& pos);
	void remove_library(BP::Library* l);
	void remove_node(BP::Node* n);

	void duplicate_selected();
	void delete_selected();

	void link_test_nodes();

	void update_gv();
	bool generate_graph_image();
	bool auto_set_layout();

	bool create(const char* filename);
	void load(const std::filesystem::path& filepath);
};

extern MyApp app;
