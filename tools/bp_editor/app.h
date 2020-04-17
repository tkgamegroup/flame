#pragma once

#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/utils/app.h>
#include <flame/utils/2d_editor.h>

using namespace flame;
using namespace graphics;

struct cEditor : Component
{
	utils::_2DEditor edt;

	BP::Slot* dragging_slot;
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
	void show_add_node_menu(const Vec2f& pos);
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

	bool create(const char* filename);
};

extern MyApp app;

BP::Node* _add_node(const std::string& type, const std::string& id, const Vec2f& pos);
void _remove_node(BP::Node* n);
std::vector<BP::Node*> _duplicate_nodes(const std::vector<BP::Node*>& models);
void _set_link(BP::Slot* in, BP::Slot* out);
