#pragma once

#include "app.h"

struct SearchView : View
{
	enum Mode
	{
		ModeSearchInText,
		ModeReplaceInText,
		ModeSearchBpParttern,
		ModeReplaceBpParttern
	};

	struct BpParttern
	{
		struct Slot
		{
			std::string name;
			uint name_hash;
			TypeInfo* type;
		};

		struct Node
		{
			std::string name;
			uint name_hash;
			std::vector<std::unique_ptr<Slot>> inputs;
			std::vector<std::unique_ptr<Slot>> outputs;
		};

		struct Link
		{
			Slot* from_slot;
			Slot* to_slot;
		};

		std::vector<std::unique_ptr<Node>> nodes;
		std::vector<std::unique_ptr<Link>> links;
	};

	struct BlueprintResult
	{
		struct GroupResult
		{
			struct NodeResult
			{
				std::string name;
				uint id;
			};

			struct PartternResult
			{
				std::string name;
				std::vector<uint> node_ids;
			};

			std::string name;
			uint name_hash;
			std::vector<NodeResult> node_results;
		};

		std::filesystem::path path;
		std::string path_str;
		std::vector<GroupResult> group_results;
	};

	struct SheetResult
	{
		std::filesystem::path path;
		std::string path_str;
	};

	Mode mode = ModeSearchInText;
	bool search_in_blueprints = true;
	bool search_in_sheets = false;
	bool search_in_names = true;
	bool search_in_values = true;
	bool match_case = false;
	bool match_whole_word = false;
	std::string find_str;
	std::string replace_str;
	BpParttern bp_find_parttern;
	BpParttern bp_replace_parttern;
	ax::NodeEditor::Detail::EditorContext* ax_editor_find = nullptr;
	ax::NodeEditor::Detail::EditorContext* ax_editor_replace = nullptr;
	std::vector<BlueprintResult> blueprint_results;
	std::vector<SheetResult> sheet_results;

	SearchView();
	SearchView(const std::string& name);
	~SearchView();
	void on_draw() override;
};

struct SearchWindow : Window
{
	SearchWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern SearchWindow search_window;
