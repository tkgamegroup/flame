#pragma once

#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/utils/app.h>

using namespace flame;
using namespace graphics;

struct cEditor : Component
{
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
	void on_id_changed(BP::Node* n);
	void on_pos_changed(BP::Node* n);
	void on_add_node(BP::Node* n);
	void on_remove_node(BP::Node* n);
	void on_data_changed(BP::Slot* s);
	void base_scale(int v);
	void base_move(const Vec2f& p);
	void show_add_node_menu(const Vec2f& pos);
	void clear_failed_flags();
	void set_failed_flags();
};

struct cDetail : Component
{
	cDetail();
	virtual ~cDetail() override;
};

struct cPreview : Component
{
	cPreview();
	virtual ~cPreview() override;
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

struct NodeDesc
{
	std::string type;
	std::string id;
	Vec2f pos;
};

struct InputSaving
{
	std::string data;
	std::string link;
};

struct OutputSaving
{
	std::vector<std::string> links;
};

struct NodeSaving
{
	NodeDesc desc;
	std::vector<InputSaving> inputs;
	std::vector<OutputSaving> outputs;
};

struct MyApp : App
{
	std::filesystem::path filepath;
	std::filesystem::path fileppath;
	BP* bp;
	bool failed;
	bool changed;
	bool auto_update;

	std::vector<BP::Node*> selected_nodes;
	std::vector<BP::Slot*> selected_links;

	cCheckbox* c_auto_update;

	cEditor* editor;
	cDetail* detail;
	cPreview* preview;
	cConsole* console;

	Entity* e_notification;

	MyApp()
	{
		bp = nullptr;
		failed = false;
		changed = false;

		editor = nullptr;
		console = nullptr;
		detail = nullptr;
		preview = nullptr;

		auto_update = false;
	}

	void deselect();
	void select(const std::vector<BP::Node*>& nodes);
	void select(const std::vector<BP::Slot*>& links);

	void set_changed(bool v);

	BP::Library* add_library(const std::wstring& filename);
	void remove_library(BP::Library* l);
	BP::Node* add_node(const NodeDesc& desc);
	void remove_nodes(const std::vector<BP::Node*> nodes);
	void set_node_id(BP::Node* n, const std::string& id);
	void set_nodes_pos(const std::vector<BP::Node*>& nodes, const std::vector<Vec2f>& pos);
	void set_links(const std::vector<std::pair<BP::Slot*, BP::Slot*>>& links);
	void set_data(BP::Slot* input, void* data, bool from_editor);

	void update();

	void save();

	void update_gv();
	bool generate_graph_image();
	bool auto_set_layout();

	void show_test_render_target(BP::Node* n);

	bool create(const char* filename);
};

extern MyApp app;
