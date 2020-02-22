#pragma once

#include <flame/serialize.h>
#include <flame/universe/ui/utils.h>
#include <flame/utils/app.h>

using namespace flame;
using namespace graphics;

struct cEditor : Component
{
	cText* tab_text;
	Vec2f add_pos;
	Entity* e_base;

	BP::Slot* dragging_slot;

	bool running;

	cEditor();
	virtual ~cEditor() override;
	void on_deselect();
	void on_select();
	void set_add_pos_center();
	void on_changed();
	void on_load();
	void on_add_library(BP::Library* l);
	void on_add_node(BP::Node* n);
	void on_add_subgraph(BP::SubGraph* s);
	void on_remove_library(BP::Library* l);
	void on_remove_node(BP::Node* n);
	void on_remove_subgraph(BP::SubGraph* s);
	void on_data_changed(BP::Slot* s);
};

struct cConsole : Component
{
	cText* c_text_log;
	cEdit* c_edit_input;

	cConsole();
	virtual ~cConsole() override;
};

enum SelType
{
	SelAir,
	SelLibrary,
	SelSubGraph,
	SelNode,
	SelSlot,
	SelLink
};

struct MyApp : App
{
	std::filesystem::path filepath;
	std::filesystem::path fileppath;
	BP* bp;
	bool changed;
	bool locked;
	bool running;

	SelType sel_type;

	union
	{
		void* plain;
		BP::Library* library;
		BP::SubGraph* subgraph;
		BP::Node* node;
		BP::Slot* slot;
		BP::Slot* link;
	}selected;

	cEditor* bp_editor;
	cConsole* console;

	Entity* e_add_node_menu;
	cEdit* add_node_menu_filter;

	MyApp()
	{
		bp_editor = nullptr;
		console = nullptr;

		bp = nullptr;
		changed = false;
		locked = false;

		sel_type = SelAir;
		selected.plain = nullptr;
		running = false;
	}

	void set_changed(bool v);

	void deselect();
	void select(SelType t, void* p);

	BP::Library* add_library(const wchar_t* filename);
	BP::Node* add_node(const char* type_name, const char* id);
	BP::SubGraph* add_subgraph(const wchar_t* filename, const char* id);
	void remove_library(BP::Library* l);
	bool remove_node(BP::Node* n);
	void remove_subgraph(BP::SubGraph* s);

	void duplicate_selected();
	void delete_selected();

	void reset_add_node_menu_filter();
	void refresh_add_node_menu();

	void link_test_nodes();

	void update_gv();
	bool generate_graph_image();
	bool auto_set_layout();

	void on_frame() override;

	void create();
	void load(const std::filesystem::path& filepath);
};

extern MyApp app;
