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
			View::name = std::format("{}###{}", sheet_path.filename().string(), std::string(sp[0]));
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
	if (!sheet)
		return;
	if (!unsaved)
		return;

	if (sheet->filename.native().starts_with(app.project_static_path.native()))
	{
		if (sheet->name.empty())
		{
			sheet->name = sheet->filename.filename().stem().string();
			sheet->name_hash = sh(sheet->name.c_str());
		}
	}
	sheet->save();
	unsaved = false;
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
		if (ImGui::ToolButton("Edit Columns", editing_columns))
			editing_columns = !editing_columns;
		ImGui::SameLine();
		if (ImGui::ToolButton("Vertical", vertical_mode))
			vertical_mode = !vertical_mode;
		ImGui::SameLine();
		if (ImGui::Button("Save"))
			save_sheet();
		if (sheet->columns.size() > 0 && sheet->columns.size() < 64)
		{
			auto manipulate_data = [](TypeInfo* type, void* data) {
				auto text_height = [](const std::string& s) {
					return ImGui::GetFontSize() * (1 + std::count(s.begin(), s.end(), '\n'));
				};

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
							if (ti->is_signed)
								ImGui::DragScalar("", ImGuiDataType_S32, &((int*)data)[i]);
							else
								ImGui::DragScalar("", ImGuiDataType_U32, &((uint*)data)[i]);
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
						ImGui::InputTextMultiline("", (std::string*)data, vec2(ImGui::GetContentRegionAvail().x - 3, text_height(*(std::string*)data) + 6));
						changed |= ImGui::IsItemDeactivatedAfterEdit();
						break;
					case DataWString:
						{
							auto s = w2s(*(std::wstring*)data);
							ImGui::InputTextMultiline("", &s, vec2(ImGui::GetContentRegionAvail().x - 3, text_height(s) + 6));
							changed |= ImGui::IsItemDeactivatedAfterEdit();
							if (changed)
								*(std::wstring*)data = s2w(s);
						}
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
				else if (type == TypeInfo::get<StrAndHash>())
				{
					ImGui::SetNextItemWidth(-1.f);
					auto& snh = *(StrAndHash*)data;
					ImGui::InputText("", &snh.s);
					changed |= ImGui::IsItemDeactivatedAfterEdit();
					if (changed)
						snh.h = sh(snh.s.c_str());
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
					ImGui::TableSetupScrollFreeze(1, 0);

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
						ImGui::PushID(i);
						if (!editing_columns)
							ImGui::TableHeader(column.name.c_str());
						else
						{
							auto name = column.name;
							ImGui::Button(graphics::font_icon_str("bars"_h).c_str());
							if (ImGui::BeginDragDropSource())
							{
								ImGui::SetDragDropPayload("Column", &i, sizeof(int));
								ImGui::TextUnformatted(name.c_str());
								ImGui::EndDragDropSource();
							}
							if (ImGui::BeginDragDropTarget())
							{
								if (auto payload = ImGui::AcceptDragDropPayload("Column"); payload)
								{
									auto column_idx = *(int*)payload->Data;
									add_event([this, column_idx, i]() {
										sheet->reorder_columns(column_idx, i);
										return false;
									});
								}
								ImGui::EndDragDropTarget();
							}
							ImGui::SameLine();
							ImGui::SetNextItemWidth(100.f);
							ImGui::InputText("##name", &name);
							if (ImGui::IsItemDeactivatedAfterEdit())
							{
								auto type = column.type;
								add_event([this, i, name, type]() {
									sheet->alter_column(i, name, type);
									return false;
								});
								unsaved = true;
							}
							ImGui::SameLine();
							ImGui::SetNextItemWidth(100.f);
							if (ImGui::BeginCombo("##type", ti_str(column.type).c_str()))
							{
								if (auto type = show_types_menu(); type)
								{
									add_event([this, i, name, type]() {
										sheet->alter_column(i, name, type);
										return false;
									});
									unsaved = true;
								}
								ImGui::EndCombo();
							}
							ImGui::SameLine();
							if (ImGui::Button(graphics::font_icon_str("trash"_h).c_str()))
							{
								add_event([this, i]() {
									sheet->remove_column(i);
									return false;
								});
								unsaved = true;
							}
						}
						ImGui::PopID();
						column.width = ImGui::GetContentRegionAvail().x;
					}
					if (editing_columns)
					{
						ImGui::TableSetColumnIndex(sheet->columns.size());
						if (ImGui::Button("New"))
						{
							add_event([this]() {
								sheet->insert_column("new_column", TypeInfo::get<float>());
								return false;
							});
							unsaved = true;
						}
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
							ImGui::Button(graphics::font_icon_str("bars"_h).c_str());
							if (ImGui::BeginDragDropSource())
							{
								ImGui::SetDragDropPayload("Row", &i, sizeof(int));
								ImGui::TextUnformatted(str(i).c_str());
								ImGui::EndDragDropSource();
							}
							if (ImGui::BeginDragDropTarget())
							{
								if (auto payload = ImGui::AcceptDragDropPayload("Row"); payload)
								{
									auto row_idx = *(int*)payload->Data;
									add_event([this, row_idx, i]() {
										sheet->reorder_rows(row_idx, i);
										return false;
									});
								}
								ImGui::EndDragDropTarget();
							}
							ImGui::SameLine();
							if (ImGui::Button(graphics::font_icon_str("trash"_h).c_str()))
							{
								add_event([this, i]() {
									sheet->remove_row(i);
									return false;
								});
								unsaved = true;
								break;
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
			if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl) && ImGui::IsKeyPressed((ImGuiKey)Keyboard_S))
				save_sheet();
		}
	}

	ImGui::End();
	if (!opened)
		delete this;
}

std::string SheetView::get_save_name()
{
	if (sheet_path.empty())
		return name;
	if (vertical_mode)
		return sheet_path.string() + "#V##Sheet";
	return sheet_path.string() + "##Sheet";
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
