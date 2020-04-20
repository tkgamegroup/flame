#pragma once

#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/utils/app.h>
#include <flame/utils/2d_editor.h>

using namespace flame;
using namespace graphics;

inline const wchar_t* type_prefix(TypeTag t, bool is_array = false)
{
	switch (t)
	{
	case TypeEnumSingle:
		return L"Enum Single\n";
	case TypeEnumMulti:
		return L"Enum Multi\n";
	case TypeData:
		return is_array ? L"Array\n" : L"Data\n";
	case TypePointer:
		return is_array ? L"Array Pointer\n" : L"Pointer\n";
	}
	return L"";
}

inline const wchar_t* node_type_prefix(char t)
{
	switch (t)
	{
	case 'S':
		return type_prefix(TypeEnumSingle);
	case 'M':
		return type_prefix(TypeEnumMulti);
	case 'V':
		return L"Variable\n";
	case 'A':
		return L"Array\n";
	}
	return L"";
}

inline Vec4c type_color(TypeTag t)
{
	switch (t)
	{
	case TypeEnumSingle:
		return Vec4c(23, 160, 93, 255);
	case TypeEnumMulti:
		return Vec4c(23, 160, 93, 255);
	case TypeData:
		return Vec4c(40, 58, 228, 255);
	case TypePointer:
		return Vec4c(239, 94, 41, 255);
	}
	return Vec4c(0);
}

inline Vec4c node_type_color(char t)
{
	switch (t)
	{
	case 'S':
		return type_color(TypeEnumSingle);
	case 'M':
		return type_color(TypeEnumMulti);
	case 'V':
		return Vec4c(0, 128, 0, 255);
	case 'A':
		return Vec4c(0, 0, 255, 255);
	}
	return Vec4c(128, 60, 220, 255);
}

struct cEditor : Component
{
	utils::_2DEditor edt;

	BP::Slot* dragging_slot;
	Vec2f dragging_slot_pos;

	cEditor();
	virtual ~cEditor() override;
	void on_before_select();
	void on_after_select();
	void on_pos_changed(BP::Node* n);
	void on_add_node(BP::Node* n);
	void on_remove_node(BP::Node* n);
	void on_data_changed(BP::Slot* s);
	void show_add_node_menu(const Vec2f& pos);
};

struct cDetail : Component
{
	Entity* e_page;

	cDetail();
	virtual ~cDetail() override;
	void on_after_select();
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
	std::string id;
	std::string type;
	BP::ObjectType object_type;
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

	Entity* e_test;

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

	void select();
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

BP::Node* _add_node(BP::ObjectType object_type, const std::string& id, const std::string& type, const Vec2f& pos);
void _remove_node(BP::Node* n);
std::vector<BP::Node*> _duplicate_nodes(const std::vector<BP::Node*>& models);
void _set_link(BP::Slot* in, BP::Slot* out);
