#include "sheet_window.h"
#include "blueprint_window.h"

#include <flame/foundation/sheet.h>
#include <flame/foundation/blueprint.h>

SheetWindow sheet_window;

SheetView::SheetView() :
	SheetView(sheet_window.views.empty() ? "Sheet" : "Sheet##" + str(rand()))
{
}

SheetView::SheetView(const std::string& name) :
	View(&sheet_window, name)
{
	auto sp = SUS::split(name, '#');
	if (sp.size() > 1)
	{
		sheet_path = sp[0];
		if (sp.size() > 2)
		{
			if (sp[1] == "V")
				vertical_mode = true;
			View::name = std::string(sp[0]) + "##Sheet";
		}
	}
}

SheetView::~SheetView()
{
	if (app_exiting)
		return;
	if (sheet)
		Sheet::release(sheet);
}

void SheetView::save_sheet()
{
	if (unsaved)
	{
		if (sheet->filename.native().starts_with(app.project_static_path.native()))
		{
			if (sheet->name.empty())
			{
				sheet->name = sheet->filename.filename().stem().string();
				sheet->name_hash = sh(sheet->name.c_str());
			}
			app.rebuild_typeinfo();
		}
		sheet->save();

		unsaved = false;
	}
}

void SheetView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened, unsaved ? ImGuiWindowFlags_UnsavedDocument : 0);
	imgui_window = ImGui::GetCurrentWindow();

	if (!sheet)
	{
		sheet = Sheet::get(sheet_path);
		//load_frame = frame;
	}
	if (sheet)
	{
		if (ImGui::ToolButton("Vertical", vertical_mode))
			vertical_mode = !vertical_mode;
		ImGui::SameLine();
		if (ImGui::Button("Save"))
			save_sheet();
		ImGui::SameLine();
		if (ImGui::Button("New Column"))
		{
			struct NewColumnDialog : ImGui::Dialog
			{
				SheetView* view;
				SheetPtr sheet;
				std::string name;
				TypeInfo* type;

				static void open(SheetView* view)
				{
					auto dialog = new NewColumnDialog;
					dialog->title = "New Column";
					dialog->view = view;
					dialog->sheet = view->sheet;
					dialog->name = "new_column " + str(dialog->sheet->columns.size() + 1);
					dialog->type = TypeInfo::get<float>();
					Dialog::open(dialog);
				}

				void draw() override
				{
					if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
					{
						ImGui::InputText("Name", &name);
						if (ImGui::BeginCombo("Type", ti_str(type).c_str()))
						{
							if (auto t = show_types_menu(); t)
								type = t;
							ImGui::EndCombo();
						}

						if (ImGui::Button("OK"))
						{
							sheet->insert_column(name, type);
							view->unsaved = true;
							close();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
							close();
						ImGui::EndPopup();
					}
				}
			};
			NewColumnDialog::open(this);
		}
		ImGui::SameLine();
		if (ImGui::Button("Alter Column"))
		{
			struct AlterColumnDialog : ImGui::Dialog
			{
				SheetView* view;
				SheetPtr sheet;
				int column_idx = 0;
				std::vector<std::string> new_names;
				std::vector<TypeInfo*> new_types;

				static void open(SheetView* view)
				{
					auto dialog = new AlterColumnDialog;
					dialog->title = "Alter Column";
					dialog->view = view;
					auto sheet = view->sheet;
					dialog->sheet = sheet;
					dialog->new_names.resize(sheet->columns.size());
					for (auto i = 0; i < sheet->columns.size(); i++)
						dialog->new_names[i] = sheet->columns[i].name;
					dialog->new_types.resize(sheet->columns.size());
					for (auto i = 0; i < sheet->columns.size(); i++)
						dialog->new_types[i] = sheet->columns[i].type;
					Dialog::open(dialog);
				}

				void draw() override 
				{
					if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
					{
						ImGui::BeginGroup();
						for (auto i = 0; i < sheet->columns.size(); i++)
						{
							auto& column = sheet->columns[i];
							ImGui::RadioButton(column.name.c_str(), &column_idx, i);
						}
						ImGui::EndGroup();

						if (!sheet->columns.empty())
						{
							ImGui::SameLine();
							ImGui::BeginGroup();
							ImGui::InputText("Name", &new_names[column_idx]);
							if (ImGui::BeginCombo("Type", ti_str(new_types[column_idx]).c_str()))
							{
								if (auto type = show_types_menu(); type)
									new_types[column_idx] = type;
								ImGui::EndCombo();
							}
							ImGui::EndGroup();
						}

						ImGui::Separator();
						if (ImGui::Button("OK"))
						{
							auto changed = false;
							for (auto i = 0; i < sheet->columns.size(); i++)
							{
								if (sheet->columns[i].name != new_names[i] || sheet->columns[i].type != new_types[i])
								{
									auto old_name_hash = sheet->columns[i].name_hash;
									auto new_name_hash = sh(new_names[i].c_str());
									sheet->alter_column(i, new_names[i], new_types[i]);

									app.update_sheet_references(sheet, old_name_hash, sheet->name_hash, new_name_hash);

									changed = true;
								}
							}
							if (changed)
								view->unsaved = true;
							close();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
							close();

						ImGui::EndPopup();
					}
				}
			};
			AlterColumnDialog::open(this);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove Column"))
		{
			struct RemoveColumnDialog : ImGui::Dialog
			{
				SheetView* view;
				SheetPtr sheet;
				int column_idx = 0;

				static void open(SheetView* view)
				{
					auto dialog = new RemoveColumnDialog;
					dialog->title = "Remove Column";
					dialog->view = view;
					dialog->sheet = view->sheet;
					Dialog::open(dialog);
				}

				void draw() override
				{
					if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
					{
						for (auto i = 0; i < sheet->columns.size(); i++)
						{
							auto& column = sheet->columns[i];
							ImGui::RadioButton(column.name.c_str(), &column_idx, i);
						}

						ImGui::Separator();
						if (ImGui::Button("OK"))
						{
							sheet->remove_column(column_idx);
							view->unsaved = true;
							close();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
							close();

						ImGui::EndPopup();
					}
				}
			};
			RemoveColumnDialog::open(this);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reorder Columns"))
		{
			struct ReorderColumnsDialog : ImGui::Dialog
			{
				SheetView* view;
				SheetPtr sheet;
				int column_idx = 0;
				int new_idx = 0;

				static void open(SheetView* view)
				{
					auto dialog = new ReorderColumnsDialog;
					dialog->title = "Reorder Columns";
					dialog->view = view;
					dialog->sheet = view->sheet;
					Dialog::open(dialog);
				}

				void draw() override
				{
					if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking))
					{
						ImGui::BeginGroup();
						for (auto i = 0; i < sheet->columns.size(); i++)
						{
							auto& column = sheet->columns[i];
							ImGui::RadioButton(column.name.c_str(), &column_idx, i);
						}
						ImGui::EndGroup();

						ImGui::SameLine();
						ImGui::BeginGroup();
						if (ImGui::BeginCombo("New Index", str(new_idx).c_str()))
						{
							for (auto i = 0; i < sheet->columns.size(); i++)
							{
								auto& column = sheet->columns[i];
								if (ImGui::Selectable(str(i).c_str(), i == new_idx))
									new_idx = i;
							}
							ImGui::EndCombo();
						}
						if (ImGui::Button(graphics::font_icon_str("arrow-left"_h).c_str()))
						{
							if (new_idx > 0)
								new_idx--;
						}
						ImGui::SameLine();
						ImGui::Text("%d", new_idx);
						ImGui::SameLine();
						if (ImGui::Button(graphics::font_icon_str("arrow-right"_h).c_str()))
						{
							if (new_idx < sheet->columns.size() - 1)
								new_idx++;
						}
						ImGui::EndGroup();

						if (!sheet->columns.empty())
						{
							ImGui::SameLine();
							ImGui::BeginGroup();
							std::vector<int> indices;
							for (auto i = 0; i < sheet->columns.size(); i++)
								indices.push_back(i);
							if (column_idx < new_idx)
								std::rotate(indices.begin() + column_idx, indices.begin() + column_idx + 1, indices.begin() + new_idx + 1);
							else
								std::rotate(indices.begin() + new_idx, indices.begin() + column_idx, indices.begin() + column_idx + 1);
							for (auto i : indices)
								ImGui::TextUnformatted(sheet->columns[i].name.c_str());
							ImGui::EndGroup();
						}

						ImGui::Separator();
						if (ImGui::Button("OK"))
						{
							sheet->reorder_columns(column_idx, new_idx);
							view->unsaved = true;
							close();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
							close();

						ImGui::EndPopup();
					}
				}
			};
			ReorderColumnsDialog::open(this);
		}
		if (sheet->columns.size() > 0 && sheet->columns.size() < 64)
		{
			auto manipulate_data = [](TypeInfo* type, void* data) {
				auto changed = false;
				if (type->tag == TagD)
				{
					auto ti = (TypeInfo_Data*)type;
					switch (ti->data_type)
					{
					case DataBool:
						ImGui::SetNextItemWidth(100.f);
						changed |= ImGui::Checkbox("", (bool*)data);
						break;
					case DataInt:
						ImGui::PushMultiItemsWidths(ti->vec_size, ImGui::GetContentRegionAvail().x);
						for (int i = 0; i < ti->vec_size; i++)
						{
							ImGui::PushID(i);
							if (i > 0)
								ImGui::SameLine();
							ImGui::DragScalar("", ImGuiDataType_S32, &((int*)data)[i]);
							changed |= ImGui::IsItemDeactivatedAfterEdit();
							ImGui::PopID();
							ImGui::PopItemWidth();
						}
						break;
					case DataFloat:
						ImGui::PushMultiItemsWidths(ti->vec_size, ImGui::GetContentRegionAvail().x);
						for (int k = 0; k < ti->vec_size; k++)
						{
							ImGui::PushID(k);
							if (k > 0)
								ImGui::SameLine();
							ImGui::DragScalar("", ImGuiDataType_Float, &((float*)data)[k], 0.01f);
							changed |= ImGui::IsItemDeactivatedAfterEdit();
							ImGui::PopID();
							ImGui::PopItemWidth();
						}
						break;
					case DataString:
						ImGui::PushItemWidth(-FLT_MIN);
						ImGui::InputText("", (std::string*)data);
						changed |= ImGui::IsItemDeactivatedAfterEdit();
						ImGui::PopItemWidth();
						break;
					case DataWString:
						ImGui::PushItemWidth(-FLT_MIN);
						{
							auto s = w2s(*(std::wstring*)data);
							ImGui::InputText("", &s);
							changed |= ImGui::IsItemDeactivatedAfterEdit();
							if (changed)
								*(std::wstring*)data = s2w(s);
						}
						ImGui::PopItemWidth();
						break;
					case DataPath:
						ImGui::PushItemWidth(-20.f);
						{
							auto s = (*(std::filesystem::path*)data).string();
							ImGui::InputText("", &s, ImGuiInputTextFlags_ReadOnly);
							if (ImGui::BeginDragDropTarget())
							{
								if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
								{
									*(std::filesystem::path*)data = Path::reverse(std::wstring((wchar_t*)payload->Data));
									changed = true;
								}
								ImGui::EndDragDropTarget();
							}
							ImGui::SameLine();
							if (ImGui::Button(graphics::font_icon_str("xmark"_h).c_str()))
							{
								*(std::filesystem::path*)data = L"";
								changed = true;
							}
						}
						break;
					}
				}
				return changed;
			};

			if (vertical_mode && sheet->rows.size() <= 1)
			{
				if (ImGui::BeginTable("##main", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoSavedSettings, ImVec2(0.f, -30.f)))
				{
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200.f);
					ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 200.f);

					auto& row = sheet->rows[0];
					for (auto i = 0; i < sheet->columns.size(); i++)
					{
						ImGui::TableNextRow();
						auto& column = sheet->columns[i];

						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(column.name.c_str());
						ImGui::TableSetColumnIndex(1);

						ImGui::PushID(i);
						auto type = column.type;
						auto data = row.datas[i];
						auto changed = manipulate_data(type, data);
						if (changed)
							unsaved = true;
						ImGui::PopID();
					}
					ImGui::EndTable();
				}
			}
			else
			{
				if (ImGui::BeginTable("##main", sheet->columns.size() + 1, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoSavedSettings, ImVec2(0.f, -30.f)))
				{
					for (auto i = 0; i < sheet->columns.size(); i++)
					{
						auto& column = sheet->columns[i];
						ImGui::TableSetupColumn(column.name.c_str(), ImGuiTableColumnFlags_WidthFixed, column.width);
					}
					ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.f);

					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (auto i = 0; i < sheet->columns.size(); i++)
					{
						auto& column = sheet->columns[i];
						ImGui::TableSetColumnIndex(i);
						ImGui::TableHeader(column.name.c_str());
						column.width = ImGui::GetContentRegionAvail().x;
					}

					for (auto i = 0; i < sheet->rows.size(); i++)
					{
						auto& row = sheet->rows[i];

						ImGui::TableNextRow();

						ImGui::PushID(i);
						for (auto j = 0; j < sheet->columns.size(); j++)
						{
							ImGui::TableSetColumnIndex(j);
							ImGui::PushID(j);
							auto type = sheet->columns[j].type;
							auto data = row.datas[j];
							auto changed = manipulate_data(type, data);
							if (changed)
								unsaved = true;
							ImGui::PopID();
						}

						{
							ImGui::TableSetColumnIndex(sheet->columns.size());
							ImGui::PushID(sheet->columns.size());
							if (ImGui::Button(graphics::font_icon_str("trash"_h).c_str()))
							{
								sheet->remove_row(i);
								unsaved = true;
								ImGui::PopID(); // column

								ImGui::PopID(); // row
								break;
							}
							ImGui::SameLine();
							if (ImGui::Button(graphics::font_icon_str("arrow-up"_h).c_str()))
							{
								if (i > 0)
								{
									std::swap(sheet->rows[i], sheet->rows[i - 1]);
									unsaved = true;
								}
							}
							ImGui::SameLine();
							if (ImGui::Button(graphics::font_icon_str("arrow-down"_h).c_str()))
							{
								if (i < sheet->rows.size() - 1)
								{
									std::swap(sheet->rows[i], sheet->rows[i + 1]);
									unsaved = true;
								}
							}
							ImGui::PopID();
						}

						ImGui::PopID();
					}
					ImGui::EndTable();
				}
			}
		}
		else
			ImGui::TextUnformatted("Empty Sheet");

		if (ImGui::Button("New Row"))
			sheet->insert_row();
	}

	auto& io = ImGui::GetIO();

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
	{
		if (!io.WantCaptureKeyboard)
		{
			if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_S))
				save_sheet();
		}
	}

	ImGui::End();
	if (!opened)
		delete this;
}

std::string SheetView::get_save_name()
{
	auto sp = SUS::split(name, '#');
	if (sp.size() == 2)
	{
		if (vertical_mode)
			return std::string(sp[0]) + "#V##Sheet";
		return std::string(sp[0]) + "##Sheet";
	}
	return name;
}

SheetWindow::SheetWindow() :
	Window("Sheet")
{
}

View* SheetWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new SheetView;
	return nullptr;
}

View* SheetWindow::open_view(const std::string& name)
{
	return new SheetView(name);
}
