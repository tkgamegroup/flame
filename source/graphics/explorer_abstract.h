#pragma once

#include "../xml.h"
#include "gui.h"

namespace flame
{
	namespace graphics
	{
		struct ExplorerAbstract
		{
			struct FolderTreeNode
			{
				bool read = false;
				std::filesystem::path path;
				FolderTreeNode* parent = nullptr;
				std::vector<std::unique_ptr<FolderTreeNode>> children;

				std::string name;

				bool peeding_open = false;

				FolderTreeNode(const std::filesystem::path& path) :
					FolderTreeNode(path.filename().string(), path)
				{
				}

				FolderTreeNode(const std::string& name, const std::filesystem::path& path) :
					name(name),
					path(path)
				{
				}

				void read_children()
				{
					if (read)
						return;

					children.clear();
					if (std::filesystem::is_directory(path))
					{
						for (auto& it : std::filesystem::directory_iterator(path))
						{
							if (std::filesystem::is_directory(it.status()) || it.path().extension() == L".fmod")
							{
								auto c = new FolderTreeNode(it.path());
								c->parent = this;
								children.emplace_back(c);
							}
						}
					}
					else
					{
						if (path == L"This Computer")
						{
							for (auto& d : get_drives())
							{
								auto c = new FolderTreeNode(d);
								c->name = d.root_name().string();
								c->parent = this;
								children.emplace_back(c);
							}
						}
					}
					read = true;
				}

				void mark_upstream_open()
				{
					if (parent)
					{
						parent->peeding_open = true;
						parent->mark_upstream_open();
					}
				}

				FolderTreeNode* find_node(const std::vector<std::string>& stems, uint idx)
				{
					if (idx == stems.size())
						return this;
					read_children();
					for (auto& c : children)
					{
						if (c->name == stems[idx])
							return c->find_node(stems, idx + 1);
					}
					return nullptr;
				}
			};

			struct Item
			{
				inline static auto size = 64.f;

				std::filesystem::path				path;
				std::string							label;
				float								label_width;
				graphics::ImagePtr					icon = nullptr;
				void(*icon_releaser)(graphics::Image*) = nullptr;

				bool has_children = false;

				Item(const std::filesystem::path& path, const std::string& text = "") :
					path(path)
				{
					label = !text.empty() ? text : path.filename().string();
#ifdef USE_IMGUI
					auto font = ImGui::GetFont();
					auto font_size = ImGui::GetFontSize();
					const char* clipped_end;
					label_width = font->CalcTextSizeA(font_size, size, 0.f, label.c_str(), label.c_str() + label.size(), &clipped_end).x;
					if (clipped_end != label.c_str() + label.size())
					{
						auto str = label.substr(0, clipped_end - label.c_str());
						float w;
						do
						{
							if (str.size() <= 1)
								break;
							str.pop_back();
							w = font->CalcTextSizeA(font_size, 9999.f, 0.f, (str + "...").c_str()).x;
						} while (w > size);
						label = str + "...";
						label_width = w;
					}
#endif
				}

				void on_added()
				{
					if (!icon)
						icon = (ImagePtr)get_icon(path);
					if (std::filesystem::is_directory(path))
						has_children = true;
				}

				~Item()
				{
					if (!icon_releaser)
					{
						if (icon)
							release_icon((Image*)icon);
					}
					else
						icon_releaser((Image*)icon);
				}
			};

			std::unique_ptr<FolderTreeNode>		folder_tree;
			FolderTreeNode*						opened_folder = nullptr;
			std::filesystem::path				peeding_open_path;
			std::pair<FolderTreeNode*, bool>	peeding_open_node;
			std::vector<FolderTreeNode*>		folder_history;
			int									folder_history_idx = -1;

			std::vector<std::unique_ptr<Item>>	items;
			std::vector<std::filesystem::path>	selected_paths;
			std::filesystem::path				pinged_path;
			uint								ping_frame = 0;
			std::string							filter;
			bool								show_as_list = false;

			std::function<void(const std::filesystem::path&)>							select_callback;
			std::function<void(const std::filesystem::path&)>							dbclick_callback;
			std::function<void(const std::filesystem::path&)>							item_context_menu_callback;
			std::function<void(const std::filesystem::path&)>							folder_context_menu_callback;
			std::function<void(const std::filesystem::path&)>							folder_drop_callback;
			std::function<void(Item*)>													item_created_callback;
			std::function<bool(const std::filesystem::path&, std::vector<Item*>&)>		special_folder_provider;
			std::function<void(const std::filesystem::path&, const std::string& name)>	rename_callback;
			std::filesystem::path														rename_path;
			std::string																	rename_string;
			uint																		rename_start_frame;

			void reset_n(const std::vector<std::filesystem::path>& paths)
			{
				items.clear();
				opened_folder = nullptr;
				if (paths.size() == 1)
					folder_tree.reset(new FolderTreeNode(paths[0]));
				else
				{
					auto n = new FolderTreeNode(L"*");
					n->read = true;
					for (auto& p : paths)
					{
						auto s = p.wstring();
						auto sp = SUW::split(s, L'=');
						if (sp.size() == 1)
							n->children.emplace_back(new FolderTreeNode(p));
						else if (sp.size() == 2)
							n->children.emplace_back(new FolderTreeNode(w2s(std::wstring(sp[1])), sp[0]));
					}
					folder_tree.reset(n);
				}
			}

			void reset(const std::filesystem::path& path)
			{
				reset_n({ path });
			}

			FolderTreeNode* find_folder(const std::filesystem::path& _path)
			{
				if (!folder_tree)
					return nullptr;
				auto sub_find = [&](FolderTreeNode* n) {
					auto path = Path::rebase(n->path, _path);
					std::vector<std::string> stems;
					for (auto it : path)
					{
						auto stem = it.string();
						if (stem == "\\" || stem == "//")
							continue;
						stems.push_back(stem);
					}
					return n->find_node(stems, 0);
				};
				if (folder_tree->path == L"*")
				{
					for (auto& c : folder_tree->children)
					{
						auto res = sub_find(c.get());
						if (res)
							return res;
					}
					return nullptr;
				}
				return sub_find(folder_tree.get());
			}

			void open_folder(FolderTreeNode* folder, bool from_histroy = false)
			{
				if (!folder)
					folder = folder_tree.get();

				if (!from_histroy && opened_folder != folder)
				{
					auto it = folder_history.begin() + (folder_history_idx + 1);
					it = folder_history.erase(it, folder_history.end());
					folder_history.insert(it, folder);
					if (folder_history.size() > 20)
						folder_history.erase(folder_history.begin());
					else
						folder_history_idx++;
				}

				opened_folder = folder;

				std::vector<Item*> dead_items(items.size());
				for (auto i = 0; i < items.size(); i++)
					dead_items[i] = items[i].get();

				if (folder)
				{
					folder->mark_upstream_open();
					folder->read_children();

					auto processed = false;
					if (special_folder_provider)
					{
						std::vector<Item*> new_items;
						if (special_folder_provider(folder->path, new_items))
						{
							for (auto i : new_items)
							{
								if (item_created_callback)
									item_created_callback(i);
								i->on_added();
								items.emplace_back(i);
							}
							processed = true;
						}
					}
					if (!processed)
					{
						if (std::filesystem::is_directory(folder->path))
						{
							std::vector<Item*> dirs;
							std::vector<Item*> files;
							for (auto& it : std::filesystem::directory_iterator(folder->path))
							{
								if (std::filesystem::is_directory(it.status()))
									dirs.push_back(new Item(it.path()));
								else
									files.push_back(new Item(it.path()));
							}
							std::sort(dirs.begin(), dirs.end(), [](const auto& a, const auto& b) {
								return a->path < b->path;
								});
							std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
								return a->path < b->path;
								});
							for (auto i : dirs)
							{
								if (item_created_callback)
									item_created_callback(i);
								i->on_added();
								items.emplace_back(i);
							}
							for (auto i : files)
							{
								if (item_created_callback)
									item_created_callback(i);
								i->on_added();
								items.emplace_back(i);
							}
						}
					}
				}

				for (auto it = items.begin(); it != items.end();)
				{
					auto i = it->get();
					auto it2 = std::find(dead_items.begin(), dead_items.end(), i);
					if (it2 == dead_items.end())
						it++;
					else
						it = items.erase(it);
				}
			}

			void ping(const std::filesystem::path& path)
			{
				pinged_path = path;
				ping_frame = frames;
			}

			void enter_rename(const std::filesystem::path& path)
			{
				if (!rename_callback)
					return;
				rename_path = path;
				rename_string = rename_path.filename().string();
				rename_start_frame = frames;
			}

			void draw()
			{
				FolderTreeNode* scroll_here_folder = nullptr;
				if (!peeding_open_path.empty() && !peeding_open_node.first)
				{
					peeding_open_node = { find_folder(peeding_open_path), false };
					peeding_open_path.clear();

					pinged_path = L"";
				}
				else if (peeding_open_node.first)
					pinged_path = L"";
				else
				{
					if (!pinged_path.empty() && ping_frame == frames)
					{
						std::filesystem::path p;
						auto s = pinged_path.wstring();
						auto sp = SUW::split(s, '#');
						if (sp.size() > 1)
							p = sp.front();
						else
							p = pinged_path.parent_path();
						auto folder = find_folder(p);
						if (folder)
						{
							folder->mark_upstream_open();
							scroll_here_folder = folder;
							peeding_open_node = { folder, false };
						}
					}
				}
				if (peeding_open_node.first)
				{
					open_folder(peeding_open_node.first, true);
					peeding_open_node = { nullptr, false };
				}

#ifdef USE_IMGUI
				auto& io = ImGui::GetIO();
				auto& style = ImGui::GetStyle();
				auto padding = style.FramePadding;
				auto line_height = ImGui::GetTextLineHeight();

				ImVec2 content_pos;
				ImVec2 content_size;

				if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::BeginChild("folders", ImVec2(0, -2));
					if (folder_tree)
					{
						std::function<void(FolderTreeNode*)> draw_node;
						draw_node = [&](FolderTreeNode* node) {
							auto flags = opened_folder == node ? ImGuiTreeNodeFlags_Selected : 0;
							if (node->read && node->children.empty())
								flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
							else
								flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
							if (node->peeding_open)
							{
								ImGui::SetNextItemOpen(true);
								node->peeding_open = false;
							}
							auto opened = ImGui::TreeNodeEx(node->name.c_str(), flags) && !(flags & ImGuiTreeNodeFlags_Leaf);
							if (node == scroll_here_folder)
								ImGui::SetScrollHereY();
							if (ImGui::IsItemClicked())
							{
								if (opened_folder != node)
									open_folder(node);
							}
							if (opened)
							{
								node->read_children();
								for (auto& c : node->children)
									draw_node(c.get());
								ImGui::TreePop();
							}
						};
						if (folder_tree->path == L"*")
						{
							for (auto& c : folder_tree->children)
								draw_node(c.get());
						}
						else
							draw_node(folder_tree.get());
					}
					ImGui::EndChild();

					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button(font_icon_str("arrow-left"_h).c_str()))
					{
						if (folder_history_idx > 0)
						{
							folder_history_idx--;
							peeding_open_node = { folder_history[folder_history_idx], true };
						}
					}
					ImGui::SameLine();
					if (ImGui::Button(font_icon_str("arrow-right"_h).c_str()))
					{
						if (folder_history_idx + 1 < folder_history.size())
						{
							folder_history_idx++;
							peeding_open_node = { folder_history[folder_history_idx], true };
						}
					}
					ImGui::SameLine();
					if (ImGui::Button(font_icon_str("arrow-up"_h).c_str()))
					{
						if (opened_folder && opened_folder->parent)
							peeding_open_node = { opened_folder->parent, false };
					}
					if (opened_folder)
					{
						auto lv = 0; int jump_lv = -1;
						for (auto& stem : Path::reverse(opened_folder->path))
						{
							ImGui::SameLine();
							if (ImGui::SmallButton(stem.string().c_str()))
								jump_lv = lv;
							ImGui::SameLine();
							ImGui::TextUnformatted("\\");
							lv++;
						}
						if (jump_lv != -1)
						{
							jump_lv = lv - jump_lv - 1;
							auto n = opened_folder;
							while (jump_lv > 0)
							{
								n = n->parent;
								jump_lv--;
							}
							peeding_open_node = { n, false };
						}
					}
					ImGui::SameLine();
					if (auto w = ImGui::GetContentRegionAvail().x; w > 112.f)
					{
						auto bar_w = min(w, 400.f);
						auto filter_w = bar_w - 94.f;
						ImGui::SameLine(0.f, w - bar_w);
						if (!filter.empty())
						{
							auto cursor_pos = ImGui::GetCursorPos();
							ImGui::SetCursorPos(ImVec2(cursor_pos.x + filter_w - 19.f, cursor_pos.y));
							if (ImGui::SmallButton(font_icon_str("xmark"_h).c_str()))
								filter.clear();
							ImGui::SetCursorPos(cursor_pos);
						}
						ImGui::SetNextItemWidth(filter_w);
						ImGui::InputText(font_icon_str("magnifying-glass"_h).c_str(), &filter);
						ImGui::SameLine();
						if (ImGui::ToolButton(font_icon_str("grip"_h).c_str(), !show_as_list))
							show_as_list = false;
						ImGui::SameLine();
						if (ImGui::ToolButton(font_icon_str("list"_h).c_str(), show_as_list))
							show_as_list = true;
					}
					else
						ImGui::NewLine();

					content_pos = ImGui::GetCursorPos();
					content_size = ImGui::GetContentRegionAvail();
					content_size.y -= 2;

					if (!show_as_list)
					{
						if (ImGui::BeginTable("contents", clamp(uint(content_size.x / (Item::size + padding.x * 2 + style.ItemSpacing.x)), 1U, 64U), ImGuiTableFlags_ScrollY, content_size))
						{
							auto dl = ImGui::GetWindowDrawList();
							for (auto i = 0; i < items.size(); i++)
							{
								auto item = items[i].get();
								if (!filter.empty())
								{
									if (!item->path.string().contains(filter))
										continue;
								}
								auto selected = std::find(selected_paths.begin(), selected_paths.end(), item->path) != selected_paths.end();
								auto pinged = pinged_path == item->path;
								auto renaming = rename_callback && rename_path == item->path;

								ImGui::TableNextColumn();
								ImGui::PushID(i);

								auto item_size = ImVec2(Item::size + padding.x * 2, Item::size + padding.y * 2);
								if (!renaming)
									item_size.y += line_height + padding.y;
								ImGui::InvisibleButton("", item_size);
								auto p0 = ImGui::GetItemRectMin();
								auto p1 = ImGui::GetItemRectMax();
								auto hovered = ImGui::IsItemHovered();
								auto active = ImGui::IsItemActive();
								ImU32 col;
								if (active)						col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
								else if (hovered || selected)	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
								else							col = ImColor(0, 0, 0, 0);
								dl->AddRectFilled(p0, p1, col);
								if (item->icon)
									dl->AddImage(item->icon, ImVec2(p0.x + padding.x, p0.y + padding.y), ImVec2(p0.x + padding.x + Item::size, p0.y + padding.y + Item::size));
								if (pinged)
									dl->AddRect(p0, p1, ImColor(200, 200, 0));
								if (!renaming)
								{
									dl->AddText(ImVec2(p0.x + padding.x + (Item::size - item->label_width) / 2, p0.y + Item::size + padding.y * 2), ImColor(255, 255, 255),
										item->label.c_str(), item->label.c_str() + item->label.size());
								}

								if (hovered)
								{
									if (io.MouseDoubleClicked[ImGuiMouseButton_Left])
									{
										if (item->has_children)
											peeding_open_path = item->path;
										if (dbclick_callback)
											dbclick_callback(item->path);
									}
									else
									{
										if (selected)
										{
											if (rename_callback && io.MouseClicked[ImGuiMouseButton_Left])
											{
												auto mpos = ImGui::GetMousePos();
												if (mpos.y > p1.y - line_height - padding.y)
												{
													renaming = true;
													enter_rename(item->path);
												}
											}
										}
										if (io.MouseClicked[ImGuiMouseButton_Left])
										{
											if (select_callback)
												select_callback(item->path);
											else
												selected_paths = { item->path };
										}
									}
									if (pinged)
										pinged_path = L"";
								}

								if (ImGui::BeginDragDropSource())
								{
									auto str = item->path.wstring();
									ImGui::SetDragDropPayload("File", str.c_str(), sizeof(wchar_t) * (str.size() + 1));
									ImGui::TextUnformatted("File");
									ImGui::EndDragDropSource();
								}

								if (hovered)
									ImGui::SetTooltip("%s", item->path.filename().string().c_str());

								ImGui::PopID();

								if (item_context_menu_callback)
								{
									if (ImGui::BeginPopupContextItem())
									{
										item_context_menu_callback(item->path);
										ImGui::EndPopup();
									}
								}

								if (renaming)
								{
									ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
									ImGui::SetNextItemWidth(Item::size + padding.x * 2);
									if (frames == rename_start_frame)
										ImGui::SetKeyboardFocusHere();
									ImGui::InputText("##rename", &rename_string, ImGuiInputTextFlags_AutoSelectAll);
									if (frames != rename_start_frame)
									{
										if (ImGui::IsItemDeactivated() || (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
										{
											if (!ImGui::IsKeyPressed(Keyboard_Esc))
												rename_callback(rename_path, rename_string);
											rename_path = L"";
										}
									}
									ImGui::PopStyleVar();
								}
							}

							if (opened_folder && folder_context_menu_callback)
							{
								if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
								{
									folder_context_menu_callback(opened_folder->path);
									ImGui::EndPopup();
								}
							}

							ImGui::EndTable();
						}
					}
					else
					{
						ImGui::BeginChild("contents", content_size);

						auto icon_size = line_height + padding.y * 2;
						auto dl = ImGui::GetWindowDrawList();

						for (auto i = 0; i < items.size(); i++)
						{
							auto item = items[i].get();
							if (!filter.empty())
							{
								if (!item->path.string().contains(filter))
									continue;
							}

							auto selected = std::find(selected_paths.begin(), selected_paths.end(), item->path) != selected_paths.end();
							auto pinged = pinged_path == item->path;
							auto renaming = rename_callback && rename_path == item->path;

							ImGui::PushID(i);

							std::string label = "      ";
							if (!renaming)
								label += item->path.filename().string();
							ImGui::Selectable(label.c_str(), selected, ImGuiSelectableFlags_AllowItemOverlap);
							auto p0 = ImGui::GetItemRectMin();
							auto p1 = ImGui::GetItemRectMax();
							auto hovered = ImGui::IsItemHovered();
							auto active = ImGui::IsItemActive();
							if (item->icon)
								dl->AddImage(item->icon, ImVec2(p0.x, p0.y), ImVec2(p0.x + icon_size, p0.y + icon_size));
							if (pinged)
								dl->AddRect(ImVec2(p0.x + 2, p0.y + 2), ImVec2(p1.x - 2, p1.y - 2), ImColor(200, 200, 0));

							if (hovered)
							{
								if (io.MouseDoubleClicked[ImGuiMouseButton_Left])
								{
									if (item->has_children)
										peeding_open_path = item->path;
									if (dbclick_callback)
										dbclick_callback(item->path);
								}
								else
								{
									if (selected)
									{
										if (rename_callback && io.MouseClicked[ImGuiMouseButton_Left])
										{
											auto x = ImGui::GetMousePos().x;
											if (x > p0.x + icon_size && x < p0.x + icon_size + ImGui::CalcTextSize(label.c_str()).x + 4.f)
											{
												renaming = true;
												enter_rename(item->path);
											}
										}
									}
									if (io.MouseClicked[ImGuiMouseButton_Left])
									{
										if (select_callback)
											select_callback(item->path);
										else
											selected_paths = { item->path };
									}
								}
								if (pinged)
									pinged_path = L"";
							}

							if (ImGui::BeginDragDropSource())
							{
								auto str = item->path.wstring();
								ImGui::SetDragDropPayload("File", str.c_str(), sizeof(wchar_t) * (str.size() + 1));
								ImGui::TextUnformatted("File");
								ImGui::EndDragDropSource();
							}

							ImGui::PopID();

							if (item_context_menu_callback)
							{
								if (ImGui::BeginPopupContextItem())
								{
									item_context_menu_callback(item->path);
									ImGui::EndPopup();
								}
							}

							if (renaming)
							{
								ImGui::SameLine();
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
								ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
								ImGui::PushItemWidth(-1);
								if (frames == rename_start_frame)
									ImGui::SetKeyboardFocusHere();
								ImGui::InputText("##rename", &rename_string, ImGuiInputTextFlags_AutoSelectAll);
								if (frames != rename_start_frame)
								{
									if (ImGui::IsItemDeactivated() || (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
									{
										if (!ImGui::IsKeyPressed(Keyboard_Esc))
											rename_callback(rename_path, rename_string);
										rename_path = L"";
									}
								}
								ImGui::PopItemWidth(); 
								ImGui::PopStyleVar(2);
							}
						}

						if (opened_folder && folder_context_menu_callback)
						{
							if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
							{
								folder_context_menu_callback(opened_folder->path);
								ImGui::EndPopup();
							}
						}

						ImGui::EndChild();
					}

					ImGui::EndTable();
				}

				ImGui::SetCursorPos(content_pos);
				ImGui::InvisibleButton("background", content_size);
				if (io.MouseClicked[ImGuiMouseButton_Left] && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped))
				{
					if (select_callback)
						select_callback(L"");
					else
						selected_paths.clear();
				}
				if (opened_folder && folder_drop_callback)
				{
					if (ImGui::BeginDragDropTarget())
					{
						folder_drop_callback(opened_folder->path);
						ImGui::EndDragDropTarget();
					}
				}
#endif
			}
		};
	}
}
