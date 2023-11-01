#pragma once

#include "app.h"

struct SearchView : View
{
	struct BlueprintResult
	{
		struct GroupResult
		{
			struct NodeResult
			{
				std::string name;
				uint id;
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

	bool search_in_blueprints = true;
	bool search_in_sheets = false;
	bool search_in_names = true;
	bool search_in_values = true;
	bool match_case = false;
	bool match_whole_word = false;
	std::string find_str;
	std::vector<BlueprintResult> blueprint_results;
	std::vector<SheetResult> sheet_results;

	SearchView();
	SearchView(const std::string& name);
	void on_draw() override;
};

struct SearchWindow : Window
{
	SearchWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern SearchWindow search_window;
