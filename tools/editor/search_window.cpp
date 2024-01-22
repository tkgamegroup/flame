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
	ax::NodeEditor::Config ax_config;
	ax_config.UserPointer = this;
	ax_config.SettingsFile = "";
	ax_config.NavigateButtonIndex = 2;
	ax_editor_find = (ax::NodeEditor::Detail::EditorContext*)ax::NodeEditor::CreateEditor(&ax_config);
	ax_editor_replace = (ax::NodeEditor::Detail::EditorContext*)ax::NodeEditor::CreateEditor(&ax_config);
}

SearchView::~SearchView()
{
	if (ax_editor_find)
		ax::NodeEditor::DestroyEditor((ax::NodeEditor::EditorContext*)ax_editor_find);
	if (ax_editor_replace)
		ax::NodeEditor::DestroyEditor((ax::NodeEditor::EditorContext*)ax_editor_replace);
}

void SearchView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened);
	imgui_window = ImGui::GetCurrentWindow();

	ImGui::BeginTabBar("##tabs");
	if (ImGui::BeginTabItem("Text"))
	{
		if (ImGui::ToolButton("Blueprint", search_in_blueprints))
			search_in_blueprints = !search_in_blueprints;
		ImGui::SameLine();
		if (ImGui::ToolButton("Sheet", search_in_sheets))
			search_in_sheets = !search_in_sheets;
		ImGui::SameLine();
		if (ImGui::ToolButton("Names", search_in_names))
			search_in_names = !search_in_names;
		ImGui::SameLine();
		if (ImGui::ToolButton("Values", search_in_values))
			search_in_values = !search_in_values;
		ImGui::SameLine();
		if (ImGui::ToolButton("Case", match_case))
			match_case = !match_case;
		ImGui::SameLine();
		if (ImGui::ToolButton("Whole Word", match_whole_word))
			match_whole_word = !match_whole_word;

		auto do_find = false;
		if (ImGui::InputText("##find", &find_str, ImGuiInputTextFlags_EnterReturnsTrue))
			do_find = true;
		ImGui::SameLine();
		if (ImGui::Button("Find"))
			do_find = true;
		if (do_find && !find_str.empty())
		{
			blueprint_results.clear();
			sheet_results.clear();

			auto assets_path = app.project_path / L"assets";
			for (auto it : std::filesystem::recursive_directory_iterator(assets_path))
			{
				if (it.is_regular_file())
				{
					auto ext = it.path().extension();
					if (search_in_blueprints && ext == L".bp")
					{
						if (auto bp = Blueprint::get(it.path()); bp)
						{
							auto blueprint_result_idx = -1;
							for (auto& g : bp->groups)
							{
								auto group_result_idx = -1;
								for (auto& n : g->nodes)
								{
									auto name = !n->display_name.empty() ? n->display_name : n->name;
									auto ok = false;
									if (search_in_names)
									{
										if (match_whole_word)
										{
											if (match_case)
												ok = name == find_str;
											else
												ok = SUS::match_case_insensitive(name, find_str);
										}
										else
										{
											if (match_case)
												ok = name.find(find_str) != std::string::npos;
											else
												ok = SUS::find_case_insensitive(name, find_str);
										}
									}
									if (!ok)
									{
										if (search_in_values)
										{
											for (auto& i : n->inputs)
											{
												if (!i->is_linked() && i->data)
												{
													auto value_str = i->type->serialize(i->data);
													if (match_whole_word)
													{
														if (match_case)
															ok = value_str == find_str;
														else
															ok = SUS::match_case_insensitive(value_str, find_str);
													}
													else
													{
														if (match_case)
															ok = value_str.find(find_str) != std::string::npos;
														else
															ok = SUS::find_case_insensitive(value_str, find_str);
													}
													if (ok)
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
					if (search_in_sheets && ext == L".sht")
					{
						if (auto sht = Sheet::get(it.path()); sht)
						{

						}
					}
				}
			}
		}

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Replace Text"))
	{
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Parttern"))
	{
		if (ImGui::Button("Find"))
		{

		}

		ax::NodeEditor::SetCurrentEditor((ax::NodeEditor::EditorContext*)ax_editor_find);
		ImGui::BeginChild("ax_editor_find", ImVec2(0, -2));
		ax::NodeEditor::Begin("node_editor_find");

		ax::NodeEditor::End();
		ImGui::EndChild();
		ax::NodeEditor::SetCurrentEditor(nullptr);

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Replace Parttern"))
	{
		ax::NodeEditor::SetCurrentEditor((ax::NodeEditor::EditorContext*)ax_editor_find);

		ax::NodeEditor::SetCurrentEditor((ax::NodeEditor::EditorContext*)ax_editor_replace);

		ax::NodeEditor::SetCurrentEditor(nullptr);
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();

	if (!blueprint_results.empty())
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
		if (ImGui::TreeNode("Blueprints"))
		{
			for (auto& bpr : blueprint_results)
			{
				ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
				if (ImGui::TreeNode(bpr.path_str.c_str()))
				{
					for (auto& gr : bpr.group_results)
					{
						ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
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
												if (bv->imgui_window)
													ImGui::FocusWindow((ImGuiWindow*)bv->imgui_window);
												if (auto n = g->find_node_by_id(r.id); n)
												{
													add_event([n, bv]() {
														bv->navigate_to_node(n);
														return false;
													}, 0.f, 3U); // tricky: we need to wait for the blueprint to be rendered
													app.render_frames += 5; // tricky: we need these frames to wait
												}
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
	if (!blueprint_results.empty() || !sheet_results.empty())
	{
		if (ImGui::Button("Clear"))
		{
			blueprint_results.clear();
			sheet_results.clear();
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
