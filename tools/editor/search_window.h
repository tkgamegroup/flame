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

	};

	bool find_in_blueprints = true;
	bool find_in_sheets = false;
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
