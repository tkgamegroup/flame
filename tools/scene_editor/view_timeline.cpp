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
	static auto frame_width = 10.f;
	const auto left_width = 200.f;
	const auto right_width = 150.f;
	const auto bar_height = 20.f;
	const auto offset = 8.f;
	auto dl = ImGui::GetWindowDrawList();

	ImGui::BeginChild("##left", ImVec2(left_width, 0.f));
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
		auto i = 0;
		for (auto& t : app.opened_timeline->tracks)
		{
			ImGui::InvisibleButton(("##track" + str(i)).c_str(), ImVec2(left_width, bar_height));
			auto p0 = ImGui::GetItemRectMin();
			auto p1 = ImGui::GetItemRectMax();
			dl->AddRectFilled(p0, p1, ImColor(55, 55, 55));
			dl->AddText(ImVec2(p0.x + 4.f, p0.y + 2.f), ImColor(255, 255, 255), t.address.c_str());
			i++;
		}
	}
	else
		ImGui::Dummy(ImVec2(left_width, bar_height));
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("##middle", ImVec2(-right_width, 0.f));
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
	ImVec2 timeline_p0;
	ImVec2 timeline_p1;
	ImGui::InvisibleButton("##timeline", ImVec2(ImGui::GetContentRegionAvailWidth(), bar_height));
	{
		timeline_p0 = ImGui::GetItemRectMin();
		timeline_p1 = ImGui::GetItemRectMax();
		dl->AddRectFilled(timeline_p0, timeline_p1, ImColor(55, 55, 55));
		auto visible_frames_count = (timeline_p1.x - timeline_p0.x - offset) / frame_width;
		for (auto i = 0; i < visible_frames_count; i++)
		{
			auto x = timeline_p0.x + offset + i * frame_width;
			auto y = timeline_p1.y;
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
			auto x = timeline_p0.x + offset + app.timeline_current_frame * frame_width;
			dl->AddText(ImVec2(x - 4.5f, timeline_p1.y - bar_height - 4.f), ImColor(200, 200, 200), graphics::FontAtlas::icon_s("chevron-down"_h).c_str());
			dl->AddLine(ImVec2(x, timeline_p1.y), ImVec2(x, timeline_p1.y + ImGui::GetContentRegionAvail().y), ImColor(200, 200, 200));
		}

		if (ImGui::IsItemActive())
		{
			auto px = ImGui::GetMousePos().x;
			app.timeline_current_frame = max(0.f, px - timeline_p0.x - offset) / frame_width;
		}
	}
	if (app.opened_timeline)
	{
		auto i = 0;
		for (auto& t : app.opened_timeline->tracks)
		{
			ImGui::PushID(i);
			auto j = 0;
			ImGui::NewLine();
			for (auto& f : t.keyframes)
			{
				ImGui::SameLine(offset + f.time * 60.f * frame_width - 2.f);
				ImGui::InvisibleButton(("##frame" + str(j)).c_str(), ImVec2(4.f, bar_height));
				auto p0 = ImGui::GetItemRectMin();
				auto p1 = ImGui::GetItemRectMax();
				dl->AddRectFilled(p0, p1, ImColor(38, 40, 56));

				if (ImGui::IsItemActive())
				{
					auto px = ImGui::GetMousePos().x;
					f.time = (max(0.f, px - timeline_p0.x - offset) / frame_width) / 60.f;
				}
				j++;
			}
			ImGui::PopID();
			i++;
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("##right");
	if (ImGui::CollapsingHeader("Active Keyframe"))
	{

	}
	ImGui::EndChild();
}
