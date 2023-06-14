#include "selection.h"
#include "view_timeline.h"

#include <flame/universe/timeline.h>

View_Timeline view_timeline;

View_Timeline::View_Timeline() :
	GuiView("Timeline")
{
}

void View_Timeline::on_draw()
{
	auto dl = ImGui::GetWindowDrawList();

	ImGui::BeginGroup();
	ImGui::Button("Preview");
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::SmallButton(graphics::FontAtlas::icon_s("backward-fast"_h).c_str());
	ImGui::SameLine();
	ImGui::SmallButton(graphics::FontAtlas::icon_s("backward-step"_h).c_str());
	ImGui::SameLine();
	ImGui::SmallButton(graphics::FontAtlas::icon_s("play"_h).c_str());
	ImGui::SameLine();
	ImGui::SmallButton(graphics::FontAtlas::icon_s("forward-step"_h).c_str());
	ImGui::SameLine();
	ImGui::SmallButton(graphics::FontAtlas::icon_s("forward-fast"_h).c_str());
	ImGui::PopStyleVar();
	ImGui::SameLine();
	ImGui::PushItemWidth(50.f);
	ImGui::InputScalar("##frame", ImGuiDataType_U32, &app.timeline_current_frame);
	ImGui::PopItemWidth();
	ImGui::Separator();
	if (!app.timeline_recording)
	{
		if (ImGui::Button(graphics::FontAtlas::icon_s("circle-dot"_h).c_str()))
			app.timeline_start_record();
	}
	else
	{
		if (ImGui::Button(graphics::FontAtlas::icon_s("stop"_h).c_str()))
			app.timeline_stop_record();
	}
	if (app.opened_timeline)
	{

	}
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::PushItemWidth(200.f);
	if (ImGui::BeginCombo("##asset", app.opened_timeline ? app.opened_timeline->filename.filename().string().c_str() : "[None]"))
	{
		if (ImGui::Selectable("Open.."))
		{

		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::SmallButton((graphics::FontAtlas::icon_s("xmark"_h) + "##asset").c_str()))
		app.close_timeline();
	ImGui::SameLine();
	if (ImGui::SmallButton(graphics::FontAtlas::icon_s("floppy-disk"_h).c_str()))
		app.set_timeline_host(nullptr);
	ImGui::SameLine();
	ImGui::PushItemWidth(100.f);
	auto s = app.e_timeline_host ? app.e_timeline_host->name : "[None]";
	ImGui::InputText("##host", &s, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		if (auto payload = ImGui::AcceptDragDropPayload("Entity"); payload)
		{
			auto e = *(EntityPtr*)payload->Data;
			app.set_timeline_host(e);
		}
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::SmallButton((graphics::FontAtlas::icon_s("xmark"_h) + "##host").c_str()))
	{

	}
	ImGui::InvisibleButton("##timeline", ImVec2(ImGui::GetContentRegionAvailWidth(), 20.f));
	{
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		dl->AddRectFilled(p0, p1, ImColor(55, 55, 55));
		static auto frame_width = 10.f;
		const auto bar_height = 20.f;
		const auto offset = 8.f;
		auto visible_frames_count = (p1.x - p0.x - offset) / frame_width;
		for (auto i = 0; i < visible_frames_count; i++)
		{
			auto x = p0.x + offset + i * frame_width;
			auto y = p1.y;
			auto h = 4.f;
			if (i % 10 == 0)
				h = 14.f;
			else if (i % 5 == 0)
				h = 10.f;
			dl->AddLine(ImVec2(x, y), ImVec2(x, y - h), ImColor(200, 200, 200));
			if (i % 5 == 0)
				dl->AddText(ImVec2(x + 2.f, y - bar_height), ImColor(200, 200, 200), str(i).c_str());
		}
		{
			auto x = p0.x + offset + app.timeline_current_frame * frame_width;
			dl->AddText(ImVec2(x - 4.5f, p1.y - bar_height - 4.f), ImColor(200, 200, 200), graphics::FontAtlas::icon_s("chevron-down"_h).c_str());
			dl->AddLine(ImVec2(x, p1.y), ImVec2(x, p1.y + ImGui::GetContentRegionAvail().y), ImColor(200, 200, 200));
		}

		if (ImGui::IsItemActive())
		{
			auto px = ImGui::GetMousePos().x;
			app.timeline_current_frame = max(0.f, px - p0.x - offset) / frame_width;
		}
	}
	ImGui::EndGroup();
}
