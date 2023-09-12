#include "sheet_window.h"

#include <flame/foundation/sheet.h>

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
		sheet_path = sp[0];
}

void SheetView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened);

	if (!sheet)
	{
		sheet = Sheet::get(sheet_path);
		//load_frame = frame;
	}
	if (sheet)
	{
		static std::vector<float> column_offsets;
		if (ImGui::Button("Save"))
		{
			std::vector<std::pair<uint, float>> column_idx_and_offsets(column_offsets.size());
			for (auto i = 0; i < sheet->header.size(); i++)
				column_idx_and_offsets[i] = std::make_pair(i, column_offsets[i]);
			std::sort(column_idx_and_offsets.begin(), column_idx_and_offsets.end(), [](const auto& a, const auto& b) {
				return a.second < b.second;
			});
			for (auto i = 0; i < sheet->header.size(); i++)
			{
				if (i != column_idx_and_offsets[i].first)
					sheet->reposition_columns(i, column_idx_and_offsets[i].first);
			}
			sheet->save();
		}
		column_offsets.resize(sheet->header.size());
		if (sheet->header.size() > 0 && sheet->header.size() <= 64)
		{
			if (ImGui::BeginTable("##main", sheet->header.size(), ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Borders, ImVec2(-100.f, 0.f)))
			{
				for (auto i = 0; i < sheet->header.size(); i++)
				{
					auto& column = sheet->header[i];
					ImGui::TableSetupColumn(column.name.c_str(), ImGuiTableColumnFlags_WidthStretch, 200.f);
				}

				ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
				for (auto i = 0; i < sheet->header.size(); i++)
				{
					auto& column = sheet->header[i];
					ImGui::TableSetColumnIndex(i);
					column_offsets[i] = ImGui::GetCursorPosX();

					ImGui::PushID(i);

					ImGui::TableHeader("");
					ImGui::SameLine();
					if (ImGui::Button(graphics::font_icon_str("xmark"_h).c_str()))
					{
						sheet->remove_column(i);
						unsaved = true;
						break;
					}
					ImGui::InputText("##Name", &column.name);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						sheet->alter_column(i, column.name, column.type);
						unsaved = true;
					}
					if (ImGui::BeginCombo("##Type", ti_str(column.type).c_str()))
					{
						if (auto type = show_types_menu(); type)
						{
							sheet->alter_column(i, column.name, type);
							unsaved = true;
						}
						ImGui::EndCombo();
					}
					ImGui::PopID();
				}

				for (auto i = 0; i < sheet->rows.size(); i++)
				{
					auto& row = sheet->rows[i];

					ImGui::TableNextRow();

					ImGui::PushID(i);
					for (auto j = 0; j < sheet->header.size(); j++)
					{
						ImGui::TableSetColumnIndex(j);
						ImGui::PushID(j);

						auto changed = false;
						auto type = sheet->header[j].type;
						auto data = row.datas[j];
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
								for (int i = 0; i < ti->vec_size; i++)
								{
									ImGui::PushID(i);
									if (i > 0)
										ImGui::SameLine();
									ImGui::DragScalar("", ImGuiDataType_S32, &((int*)data)[i]);
									changed |= ImGui::IsItemDeactivatedAfterEdit();
									ImGui::PopID();
								}
								break;
							case DataFloat:
								for (int k = 0; k < ti->vec_size; k++)
								{
									ImGui::PushID(k);
									if (k > 0)
										ImGui::SameLine();
									ImGui::DragScalar("", ImGuiDataType_Float, &((float*)data)[k], 0.01f);
									changed |= ImGui::IsItemDeactivatedAfterEdit();
									ImGui::PopID();
								}
								break;
							case DataString:
								ImGui::InputText("", (std::string*)data);
								changed |= ImGui::IsItemDeactivatedAfterEdit();
								break;
							}
						}
						if (changed)
							unsaved = true;

						ImGui::PopID();
					}
					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}
		ImGui::SameLine();
		ImGui::BeginGroup();
		if (ImGui::Button("New Column"))
		{
			sheet->insert_column("new_column", TypeInfo::get<float>());
		}
		if (ImGui::Button("New Row"))
		{
			sheet->insert_row();
		}
		ImGui::EndGroup();
	}

	ImGui::End();
	if (!opened)
		delete this;
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
