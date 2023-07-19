#include "selection.h"
#include "timeline_window.h"

#include <flame/universe/timeline.h>

TimelineWindow timeline_window;

TimelineView::TimelineView() :
	View(&timeline_window, "Timeline##" + str(rand()))
{
}

TimelineView::TimelineView(const std::string& name) :
	View(&timeline_window, name)
{
}

void TimelineView::on_draw()
{
	bool opened = true;
	ImGui::Begin(name.c_str(), &opened);

	auto& io = ImGui::GetIO();
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
	if (ImGui::SmallButton(graphics::FontAtlas::icon_s("backward-step"_h).c_str()))
		app.set_timeline_current_frame((int)app.timeline_current_frame - 1);
	ImGui::SameLine();
	if (ImGui::SmallButton(graphics::FontAtlas::icon_s("play"_h).c_str()))
		app.timeline_toggle_playing();
	ImGui::SameLine();
	if (ImGui::SmallButton(graphics::FontAtlas::icon_s("forward-step"_h).c_str()))
		app.set_timeline_current_frame((int)app.timeline_current_frame + 1);
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

			if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::MenuItem("Remove"))
				{
					app.opened_timeline->tracks.erase(app.opened_timeline->tracks.begin() + i);
					for (auto it = selected_keyframes.begin(); it != selected_keyframes.end();)
					{
						if (it->first == i)
							it = selected_keyframes.erase(it);
						else
							it++;
					}
					break;
				}

				ImGui::EndPopup();
			}

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
	{
		if (app.opened_timeline)
			app.opened_timeline->save(app.opened_timeline->filename);
	}
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
		app.set_timeline_host(nullptr);
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
			app.set_timeline_current_frame((int)max(0.f, px - timeline_p0.x - offset) / frame_width);
		}
	}

	static auto box_select_p0 = vec2(-1.f);
	static auto box_select_p1 = vec2(-1.f);
	auto do_select = false;
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		auto p = ImGui::GetMousePos();
		if (p.x > timeline_p0.x && p.x < timeline_p1.x &&
			p.y > timeline_p1.y)
		{
			box_select_p0 = p;
			selected_keyframes.clear();
		}
	}
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		if (box_select_p0.x > 0.f)
		{
			selected_keyframes.clear();
			do_select = true;
		}
	}
	if (box_select_p0.x > 0.f)
	{
		box_select_p1 = ImGui::GetMousePos();
		dl->AddRectFilled(box_select_p0, box_select_p1, ImColor(38, 79, 120, 128));
	}
	if (app.opened_timeline)
	{
		auto i = 0;
		for (auto& t : app.opened_timeline->tracks)
		{
			ImGui::PushID(i);
			ImGui::NewLine();
			auto j = 0;
			for (auto& f : t.keyframes)
			{
				ImGui::SameLine(offset + f.time * 60.f * frame_width - 2.f);
				ImGui::InvisibleButton(("##frame" + str(j)).c_str(), ImVec2(4.f, bar_height));
				auto p0 = ImGui::GetItemRectMin();
				auto p1 = ImGui::GetItemRectMax();
				auto selected = std::find(selected_keyframes.begin(), selected_keyframes.end(), std::make_pair(i, j)) != selected_keyframes.end();
				dl->AddRectFilled(p0, p1, selected ? ImColor(255, 190, 51) : ImColor(191, 191, 191));

				if (do_select && !(p1.x < min(box_select_p0.x, box_select_p1.x) ||
					p0.x > max(box_select_p0.x, box_select_p1.x) ||
					p1.y < min(box_select_p0.y, box_select_p1.y) ||
					p0.y > max(box_select_p0.y, box_select_p1.y)))
					selected_keyframes.push_back({ i, j });
				if (ImGui::IsItemClicked())
				{
					box_select_p0 = vec2(-1.f);
					selected_keyframes.clear();
					selected_keyframes.push_back({ i, j });
				}
				if (ImGui::IsItemActive())
				{
					auto px = ImGui::GetMousePos().x;
					auto diff = int(round(max(0.f, px - timeline_p0.x - offset) / frame_width)) - int(round(f.time * 60.f));
					if (diff != 0)
					{
						auto ok = true;
						for (auto& pair : selected_keyframes)
						{
							auto& t = app.opened_timeline->tracks[pair.first];
							auto& kf = t.keyframes[pair.second];
							auto target_time = (int(round(kf.time * 60.f)) + diff) / 60.f;
							if (target_time < 0.f)
							{
								ok = false;
								break;
							}
							for (auto& kf : t.keyframes)
							{
								if (kf.time == target_time)
								{
									ok = false;
									break;
								}
							}
							if (!ok)
								break;
						}
						if (ok)
						{
							for (auto& pair : selected_keyframes)
							{
								auto& t = app.opened_timeline->tracks[pair.first];
								t.keyframes.erase(t.keyframes.begin() + pair.second);
								auto kf = t.keyframes[pair.second];
								kf.time = (int(round(kf.time * 60.f)) + diff) / 60.f;
								auto it = std::lower_bound(t.keyframes.begin(), t.keyframes.end(), kf.time, [](const auto& a, auto t) {
									return a.time < t;
								});
								pair.second = it - t.keyframes.begin();
								t.keyframes.insert(it, kf);
							}
						}
					}
				}

				j++;
			}
			ImGui::PopID();

			i++;
		}
	}
	if (do_select)
		box_select_p0 = vec2(-1.f);

	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
	{
		if (!io.WantCaptureKeyboard)
		{
			if (ImGui::IsKeyPressed(Keyboard_Del))
			{
				for (auto it = selected_keyframes.rbegin(); it != selected_keyframes.rend(); it++)
				{
					auto& pair = *it;
					auto& track = app.opened_timeline->tracks[pair.first];
					track.keyframes.erase(track.keyframes.begin() + pair.second);
					if (track.keyframes.empty())
						app.opened_timeline->tracks.erase(app.opened_timeline->tracks.begin() + pair.first);
				}
				selected_keyframes.clear();
			}
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("##right");
	if (ImGui::CollapsingHeader("Active Keyframe"))
	{
		if (selected_keyframes.empty())
			ImGui::TextUnformatted("No keyframes selected");
		else if (selected_keyframes.size() > 1)
			ImGui::TextUnformatted("Multiple keyframes selected");
		else
		{
			auto& pair = selected_keyframes[0];
			auto& kf = app.opened_timeline->tracks[pair.first].keyframes[pair.second];
			ImGui::Text("Time: %f", kf.time);
			ImGui::Text("Value: %s", kf.value.c_str());
		}
	}
	ImGui::EndChild();

	ImGui::End();
	if (!opened)
		delete this;
}

TimelineWindow::TimelineWindow() :
	Window("Timeline")
{
}

void TimelineWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		new TimelineView;
}

void TimelineWindow::open_view(const std::string& name)
{
	new TimelineView(name);
}
