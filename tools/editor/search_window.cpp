#include "search_window.h"
#include "sheet_window.h"
#include "blueprint_window.h"

#include <flame/foundation/sheet.h>
#include <flame/foundation/blueprint.h>

SearchWindow search_window;

SearchView::SearchView() :
	SearchView(search_window.views.empty() ? "Search" : "Search##" + str(rand()))
{
}

SearchView::SearchView(const std::string& name) :
	View(&search_window, name)
{
}

void SearchView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened);

	if (ImGui::ToolButton("Blueprint", find_in_blueprints))
		find_in_blueprints = !find_in_blueprints;
	ImGui::SameLine();
	if (ImGui::ToolButton("Sheet", find_in_sheets))
		find_in_sheets = !find_in_sheets;

	auto do_find = false;
	if (ImGui::InputText("##find", &find_str, ImGuiInputTextFlags_EnterReturnsTrue))
		do_find = true;;
	ImGui::SameLine();
	if (ImGui::Button("Find"))
		do_find = true;
	if (do_find)
	{
		auto find_str_lower_case = find_str;
		std::transform(find_str_lower_case.begin(), find_str_lower_case.end(), find_str_lower_case.begin(), ::tolower);

		blueprint_results.clear();
		sheet_results.clear();

		auto assets_path = app.project_path / L"assets";
		for (auto it : std::filesystem::recursive_directory_iterator(assets_path))
		{
			if (it.is_regular_file())
			{
				auto ext = it.path().extension();
				if (find_in_blueprints && ext == L".bp")
				{
					if (auto bp = Blueprint::get(it.path()); bp)
					{
						auto blueprint_result_idx = -1;
						for (auto& g : bp->groups)
						{
							auto group_result_idx = -1;
							for (auto& n : g->nodes)
							{
								auto display_name_lower_case = n->display_name;
								std::transform(display_name_lower_case.begin(), display_name_lower_case.end(), display_name_lower_case.begin(), ::tolower);
								auto ok = display_name_lower_case.contains(find_str_lower_case);
								if (!ok)
								{
									for (auto& i : n->inputs)
									{
										auto input_name_lower_case = i->name;
										std::transform(input_name_lower_case.begin(), input_name_lower_case.end(), input_name_lower_case.begin(), ::tolower);
										if (input_name_lower_case.contains(find_str_lower_case))
										{
											ok = true;
											break;
										}
										if (!i->is_linked() && i->data)
										{
											auto value_str = i->type->serialize(i->data);
											auto value_str_lower_case = value_str;
											std::transform(value_str_lower_case.begin(), value_str_lower_case.end(), value_str_lower_case.begin(), ::tolower);
											if (value_str_lower_case.contains(find_str_lower_case))
											{
												ok = true;
												break;
											}
										}
									}
								}
								if (ok)
								{
									if (blueprint_result_idx == -1)
									{
										blueprint_result_idx = blueprint_results.size();
										auto& r = blueprint_results.emplace_back();
										r.path = Path::reverse(it.path());
										r.path_str = r.path.string();
									}
									if (group_result_idx == -1)
									{
										auto& bpr = blueprint_results[blueprint_result_idx];
										group_result_idx = bpr.group_results.size();
										auto& gr = bpr.group_results.emplace_back();
										gr.name = g->name;
										gr.name_hash = g->name_hash;
									}

									auto& gr = blueprint_results[blueprint_result_idx].group_results[group_result_idx];
									auto& nr = gr.node_results.emplace_back();
									nr.name = !n->display_name.empty() ? n->display_name : n->name;
									nr.id = n->object_id;
								}
							}

						}
						Blueprint::release(bp);
					}
				}
				if (find_in_sheets && ext == L".sht")
				{

				}
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		blueprint_results.clear();
		sheet_results.clear();
	}

	if (!blueprint_results.empty())
	{
		ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
		if (ImGui::TreeNode("Blueprints"))
		{
			for (auto& bpr : blueprint_results)
			{
				ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
				if (ImGui::TreeNode(bpr.path_str.c_str()))
				{
					for (auto& gr : bpr.group_results)
					{
						ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
						if (ImGui::TreeNode(gr.name.c_str()))
						{
							for (auto& r : gr.node_results)
							{
								if (ImGui::Selectable(std::format("{} ({})", r.name, r.id).c_str()))
								{
									for (auto& v : blueprint_window.views)
									{
										auto bv = (BlueprintView*)v.get();
										if (bv->blueprint_path == bpr.path)
										{
											if (auto g = bv->blueprint->find_group(gr.name_hash); g)
											{
												if (auto n = g->find_node_by_id(r.id); n)
													bv->navigate_to_node(n);
											}
											break;
										}
									}
								}
							}
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
	if (!opened)
		delete this;
}

SearchWindow::SearchWindow() :
	Window("Search")
{
}

View* SearchWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new SearchView;
	return nullptr;
}

View* SearchWindow::open_view(const std::string& name)
{
	return new SearchView(name);
}
