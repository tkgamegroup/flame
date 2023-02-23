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

				inline FolderTreeNode(const std::filesystem::path& path) :
					FolderTreeNode(path.filename().string(), path)
				{
				}

				inline FolderTreeNode(const std::string& name, const std::filesystem::path& path) :
					name(name),
					path(path)
				{
				}

				inline void read_children()
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

				inline void mark_upstream_open()
				{
					if (parent)
					{
						parent->peeding_open = true;
						parent->mark_upstream_open();
					}
				}

				inline FolderTreeNode* find_node(const std::vector<std::string>& stems, uint idx)
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

				std::filesystem::path path;
				std::string text;
				float text_width;
				graphics::ImagePtr image = nullptr;

				bool has_children = false;

				inline Item(const std::filesystem::path& path, const std::string& _text = "") :
					path(path)
				{
					text = !_text.empty() ? _text : path.filename().string();
#if USE_IMGUI
					auto font = ImGui::GetFont();
					auto font_size = ImGui::GetFontSize();
					const char* clipped_end;
					text_width = font->CalcTextSizeA(font_size, size, 0.f, &*text.begin(), text.c_str() + text.size(), &clipped_end).x;
					if (clipped_end != text.c_str() + text.size())
					{
						auto str = text.substr(0, clipped_end - text.c_str());
						float w;
						do
						{
							if (str.size() <= 1)
								break;
							str.pop_back();
							w = font->CalcTextSizeA(font_size, 9999.f, 0.f, (str + "...").c_str()).x;
						} while (w > size);
						text = str + "...";
						text_width = w;
					}
#endif

					image = (ImagePtr)get_icon(path);

				}

				inline ~Item()
				{
					if (image)
						release_icon(path);
				}
			};

			std::unique_ptr<FolderTreeNode> folder_tree;
			FolderTreeNode* opened_folder = nullptr;
			uint open_folder_frame = 0;
			std::filesystem::path peeding_open_path;
			std::pair<FolderTreeNode*, bool> peeding_open_node;
			FolderTreeNode* peeding_scroll_here_folder = nullptr;
			std::vector<FolderTreeNode*> folder_history;
			int folder_history_idx = -1;

			std::vector<std::unique_ptr<Item>> items;
			std::filesystem::path selected_path;

			std::function<void(const std::filesystem::path&)> select_callback;
			std::function<void(const std::filesystem::path&)> dbclick_callback;
			std::function<void(const std::filesystem::path&)> item_context_menu_callback;
			std::function<void(const std::filesystem::path&)> folder_context_menu_callback;
			std::function<void(const std::filesystem::path&)> folder_drop_callback;

			inline void reset_n(const std::vector<std::filesystem::path>& paths)
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
						auto sp = SUW::split(p, '=');
						if (sp.size() == 1)
							n->children.emplace_back(new FolderTreeNode(p));
						else if (sp.size() == 2)
							n->children.emplace_back(new FolderTreeNode(w2s(sp[1]), sp[0]));
					}
					folder_tree.reset(n);
				}
			}

			inline void reset(const std::filesystem::path& path)
			{
				reset_n({ path });
			}

			inline FolderTreeNode* find_folder(const std::filesystem::path& _path)
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

			inline void open_folder(FolderTreeNode* folder, bool from_histroy = false)
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
				open_folder_frame = frames;

				items.clear();

				if (folder)
				{
					folder->mark_upstream_open();
					folder->read_children();

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
							i->has_children = true;
							items.emplace_back(i);
						}
						for (auto i : files)
						{
							if (i->path.extension() == L".fmod")
								i->has_children = true;
							items.emplace_back(i);
						}
					}
					else
					{
						auto ext = folder->path.extension();
						if (ext == L".fmod")
						{
							std::ifstream file(folder->path);
							if (file.good())
							{
								LineReader src(file);
								src.read_block("model:");
								pugi::xml_document doc;
								pugi::xml_node doc_root;
								if (doc.load_string(src.to_string().c_str()) && (doc_root = doc.first_child()).name() == std::string("model"))
								{
									for (auto n_bone : doc_root.child("bones"))
									{
										auto path = folder->path;
										path += L"#armature";
										items.emplace_back(new Item(path, "armature"));
										break;
									}
									auto idx = 0;
									for (auto n_mesh : doc_root.child("meshes"))
									{
										auto path = folder->path;
										path += L"#mesh" + wstr(idx);
										items.emplace_back(new Item(path, "mesh " + str(idx)));
										idx++;
									}
								}
							}
						}
					}
				}
			}

			inline void ping(const std::filesystem::path& path)
			{
				std::filesystem::path p;
				auto sp = SUW::split(path.wstring(), '#');
				if (sp.size() > 1)
					p = sp.front();
				else
					p = path.parent_path();
				auto folder = find_folder(p);
				if (folder)
				{
					folder->mark_upstream_open();
					peeding_scroll_here_folder = folder;
					open_folder(folder);
				}
			}

			inline void draw()
			{
				if (!peeding_open_path.empty())
				{
					open_folder(find_folder(peeding_open_path));
					peeding_open_path.clear();
				}
				if (peeding_open_node.first)
				{
					open_folder(peeding_open_node.first, true);
					peeding_open_node = { nullptr, false };
				}

#if USE_IMGUI
				if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
				{
					auto& style = ImGui::GetStyle();

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
							if (peeding_scroll_here_folder == node)
							{
								ImGui::SetScrollHereY();
								peeding_scroll_here_folder = nullptr;
							}
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
					if (ImGui::Button(FontAtlas::icon_s("arrow-left"_h).c_str()))
					{
						if (folder_history_idx > 0)
						{
							folder_history_idx--;
							peeding_open_node = { folder_history[folder_history_idx], true };
						}
					}
					ImGui::SameLine();
					if (ImGui::Button(FontAtlas::icon_s("arrow-right"_h).c_str()))
					{
						if (folder_history_idx + 1 < folder_history.size())
						{
							folder_history_idx++;
							peeding_open_node = { folder_history[folder_history_idx], true };
						}
					}
					ImGui::SameLine();
					if (ImGui::Button(FontAtlas::icon_s("arrow-up"_h).c_str()))
					{
						if (opened_folder && opened_folder->parent)
							peeding_open_node = { opened_folder->parent, false };
					}
					if (opened_folder)
					{
						ImGui::SameLine();
						ImGui::TextUnformatted(Path::reverse(opened_folder->path).string().c_str());
					}

					auto content_pos = ImGui::GetCursorPos();
					auto content_size = ImGui::GetContentRegionAvail();
					content_size.y -= 4;
					ImGui::InvisibleButton("background", content_size);
					if (opened_folder && folder_drop_callback)
					{
						if (ImGui::BeginDragDropTarget())
						{
							folder_drop_callback(opened_folder->path);
							ImGui::EndDragDropTarget();
						}
					}
					ImGui::SetCursorPos(content_pos);
					auto padding = style.FramePadding;
					auto line_height = ImGui::GetTextLineHeight();
					if (ImGui::BeginTable("contents", clamp(uint(content_size.x / (Item::size + padding.x * 2 + style.ItemSpacing.x)), 1U, 64U), ImGuiTableFlags_ScrollY, content_size))
					{
						auto just_selected = false;
						if (!items.empty())
						{
							for (auto i = 0; i < items.size(); i++)
							{
								auto item = items[i].get();

								ImGui::TableNextColumn();
								ImGui::PushID(i);

								auto selected = false;
								ImGui::InvisibleButton("", ImVec2(Item::size + padding.x * 2, Item::size + line_height + padding.y * 3));
								auto p0 = ImGui::GetItemRectMin();
								auto p1 = ImGui::GetItemRectMax();
								auto hovered = ImGui::IsItemHovered();
								auto active = ImGui::IsItemActive();
								ImU32 col;
								if (active)											col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
								else if (hovered || selected_path == item->path)	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
								else												col = ImColor(0, 0, 0, 0);
								auto draw_list = ImGui::GetWindowDrawList();
								draw_list->AddRectFilled(p0, p1, col);
								if (item->image)
									draw_list->AddImage(item->image, ImVec2(p0.x + padding.x, p0.y + padding.y), ImVec2(p1.x - padding.x, p1.y - line_height - padding.y * 2));
								draw_list->AddText(ImVec2(p0.x + padding.x + (Item::size - item->text_width) / 2, p0.y + Item::size + padding.y * 2), ImColor(255, 255, 255), item->text.c_str(), item->text.c_str() + item->text.size());

								if (frames > open_folder_frame + 3 && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hovered && ImGui::IsItemDeactivated())
								{
									selected_path = item->path;
									if (select_callback)
										select_callback(selected_path);
									selected = true;
								}
								if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && active)
								{
									if (item->has_children)
										peeding_open_path = item->path;
									else
									{
										if (dbclick_callback)
											dbclick_callback(item->path);
									}
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

								if (selected)
									just_selected = true;
								ImGui::PopID();

								if (item_context_menu_callback)
								{
									if (ImGui::BeginPopupContextItem())
									{
										item_context_menu_callback(item->path);
										ImGui::EndPopup();
									}
								}
							}
						}
						if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !just_selected)
						{
							selected_path = L"";
							if (select_callback)
								select_callback(selected_path);
						}
						if (opened_folder)
						{
							if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
							{
								if (folder_context_menu_callback)
									folder_context_menu_callback(opened_folder->path);
								else
								{
									//if (ImGui::MenuItem("New Folder"))
									//	std::filesystem::create_directory();
								}
								ImGui::EndPopup();
							}
						}
						ImGui::EndTable();
					}

					ImGui::EndTable();
				}
#endif
			}
		};
	}
}
