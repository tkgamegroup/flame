#include <filesystem>

#include <flame/global.h>
#include <flame/engine/graphics/descriptor.h>

#include "../select.h"
#include "resource_explorer.h"
#include "inspector.h"
#include "image_editor.h"
#include "model_editor.h"
#include "scene_editor.h"

ResourceExplorer *resourceExplorer = nullptr;

ResourceExplorer::ResourceExplorer() :
	FileSelector("Resource Explorer", flame::ui::FileSelectorOpen, "", 0, flame::ui::FileSelectorTreeMode)
{
}

ResourceExplorer::~ResourceExplorer()
{
	resourceExplorer = nullptr;
}

void ResourceExplorer::on_right_area_show()
{
	static char filter[260];
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	ImGui::InputText(ICON_FA_SEARCH, filter, TK_ARRAYSIZE(filter));
	ImGui::PopStyleVar();
	if (select_dir)
	{
		ImGui::BeginChild("listview", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 1));
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		const ImVec2 img_size(64.f, 64.f);
		const ImVec2 widget_size = img_size + ImVec2(0.f, 20.f);
		auto &style = ImGui::GetStyle();
		int column_count = (splitter.size[1] - style.WindowPadding.x * 2.f) / (widget_size.x + style.FramePadding.x + style.ItemSpacing.x);
		ImGui::Columns(column_count < 1 ? 1 : column_count, nullptr, false);
		auto fShow = [&](ItemData *d, bool is_folder) {
			auto pos = ImGui::GetCursorScreenPos();

			//draw_list->PushClipRect(pos, pos + widget_size, true);
			if (column_count > 1)
				pos.x += (ImGui::GetColumnWidth() - widget_size.x) * 0.5f;
			ImGui::SetCursorScreenPos(pos);
			ImGui::InvisibleButton(d->filename.c_str(), widget_size);
			int sel = 0;
			if (ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseClicked(1))
					selected = d->filename;
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (is_folder)
						select_dir = (DirItem*)d;
					else
					{
						auto f = (FileItem*)d;
						switch (f->file_type)
						{
							case flame::FileTypeImage:
							{
								new ImageEditor(flame::get_texture(f->filename));
								break;
							}
							case flame::FileTypeModel:
							{
								auto m = flame::getModel(f->filename);
								if (m)
									new ModelEditor(m);
								break;
							}
							case flame::FileTypeScene:
							{
								auto s = flame::create_scene(f->filename);
								if (s)
								{
									s->name = "scene";
									if (!scene_editor)
										scene_editor = new SceneEditor(s);
									else
										scene_editor->scene = s;
								}
								break;
							}
						}
					}
				}
				sel = 1;
			}
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("file", d->filename.c_str(), d->filename.size() + 1);
				ImGui::TextUnformatted(d->filename.c_str());
				ImGui::EndDragDropSource();
			}
			if (selected.get_filename() == d->filename)
				sel = 2;
			if (sel > 0)
				draw_list->AddRectFilled(pos, pos + widget_size, ImColor(255, 122, 50, sel == 1 ? 60 : 255));
			flame::Texture *img = nullptr;
			if (is_folder)
				img = folder_image.get();
			else
			{
				auto f = (FileItem*)d;
				if (f->file_type == flame::FileTypeImage)
					img = f->preview_image.get();
				else
					img = file_image.get();
			}
			draw_list->AddImage(ImTextureID(img->ui_index), pos, pos + img_size);
			draw_list->AddText(pos + ImVec2(0, img_size.y), ImColor(0, 0, 0), d->value.c_str());
			//draw_list->PopClipRect();
			if (column_count > 1)
				ImGui::NextColumn();
		};
		for (auto &d : select_dir->dir_list)
			fShow(d.get(), true);
		for (auto &f : select_dir->file_list)
			fShow(f.get(), false);
		ImGui::Columns(1);
		ImGui::EndChild();
	}
}
