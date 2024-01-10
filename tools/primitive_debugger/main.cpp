#include <flame/foundation/system.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/gui.h>
#include <flame/graphics/application.h>

using namespace flame;
using namespace graphics;

struct App : GraphicsApplication
{
	enum Type
	{
		TypeLines,
		TypePolygon
	};

	std::vector<std::pair<vec3, std::string>> points;
	std::vector<std::pair<Type, std::vector<int>>> primitives;
	std::vector<int> selected_points;
	int selected_primitive = -1;

	vec3 camera_pos = vec3(0.f);
	quat camera_qut = quat(1.f, 0.f, 0.f, 0.f);
	mat3 camera_rot = mat3(1.f);
	float camera_zoom = 5.f;

	void on_gui() override
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings);
		ImGui::PopStyleVar(2);

		ImGui::BeginGroup();

		if (ImGui::BeginListBox("##points", vec2(200, 200)))
		{
			for (auto i = 0; i < points.size(); i++)
			{
				ImGui::PushID(i);
				if (ImGui::Selectable(points[i].second.empty() ? str(points[i].first).c_str() : points[i].second.c_str(), 
					std::find(selected_points.begin(), selected_points.end(), i) != selected_points.end()))
				{
					if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl))
					{
						auto found = false;
						for (auto it = selected_points.begin(); it != selected_points.end();)
						{
							if (*it == i)
							{
								found = true;
								it = selected_points.erase(it);
								break;
							}
							else
								it++;
						}
						if (!found)
							selected_points.push_back(i);
					}
					else if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Shift))
					{
						auto found = false;
						for (auto i2 : selected_points)
						{
							if (i2 == i)
							{
								found = true;
								break;
							}
						}
						if (!found)
							selected_points.push_back(i);
					}
					else
						selected_points = { i };
				}
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Add Lines"))
					{
						if (!selected_points.empty())
							primitives.emplace_back(TypeLines, selected_points);
					}
					if (ImGui::MenuItem("Add Polygon"))
					{
						if (!selected_points.empty())
							primitives.emplace_back(TypePolygon, selected_points);
					}
					ImGui::EndPopup();
				}
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
		ImGui::PushID("point_buttons");
		if (ImGui::SmallButton("P"))
		{
			vec3 p;
			auto str = w2s(get_clipboard());
			sscanf_s(str.data(), "{x=%f y=%f z=%f", &p.x, &p.y, &p.z);
			points.emplace_back(p, "");
		}
		ImGui::SameLine();
		if (ImGui::SmallButton(graphics::font_icon_str("plus"_h).c_str()))
		{
			points.resize(points.size() + 1);
			selected_points = { (int)points.size() - 1 };
		}
		ImGui::SameLine();
		if (ImGui::SmallButton(graphics::font_icon_str("minus"_h).c_str()))
		{
			if (!selected_points.empty())
			{
				for (auto i = 0; i < selected_points.size(); i++)
				{
					auto idx = selected_points[i];
					points.erase(points.begin() + idx);
					for (auto& p : primitives)
					{
						for (auto it = p.second.begin(); it != p.second.end();)
						{
							if (*it == idx)
							{
								it = p.second.erase(it);
								break;
							}
							else
								it++;
						}
					}
					for (auto j = i + 1; j < selected_points.size(); j++)
					{
						if (selected_points[j] > idx)
							selected_points[j]--;
					}
				}
				selected_points.clear();
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
		{
			if (selected_points.size() == 1)
			{
				auto idx = selected_points[0];
				if (idx > 0)
				{
					std::swap(points[idx], points[idx - 1]);
					selected_points[0]--;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
		{
			if (selected_points.size() == 1)
			{
				auto idx = selected_points[0];
				if (idx < points.size() - 1)
				{
					std::swap(points[idx], points[idx + 1]);
					selected_points[0]++;
				}
			}
		}
		if (selected_points.size() == 1)
		{
			ImGui::SetNextItemWidth(200.f);
			ImGui::InputText("##name", &points[selected_points[0]].second);
			ImGui::SetNextItemWidth(200.f);
			ImGui::InputFloat3("##values", &points[selected_points[0]].first.x);
		}
		ImGui::PopID();
		if (ImGui::BeginListBox("##primitives", vec2(200, 200)))
		{
			for (auto i = 0; i < primitives.size(); i++)
			{
				ImGui::PushID(i);
				if (ImGui::Selectable(TypeInfo::serialize_t(primitives[i].second).c_str(), selected_primitive == i))
					selected_primitive = i;
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
		ImGui::PushID("primitive_buttons");
		if (ImGui::SmallButton(graphics::font_icon_str("plus"_h).c_str()))
		{
			primitives.resize(primitives.size() + 1);
			selected_primitive = primitives.size() - 1;
		}
		ImGui::SameLine();
		if (ImGui::SmallButton(graphics::font_icon_str("minus"_h).c_str()))
		{
			if (selected_primitive != -1)
			{
				primitives.erase(primitives.begin() + selected_primitive);
				if (primitives.empty())
					selected_primitive = -1;
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
		{
			if (selected_primitive > 0)
			{
				std::swap(primitives[selected_primitive], primitives[selected_primitive - 1]);
				selected_primitive--;
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
		{
			if (selected_primitive != -1 && selected_primitive < primitives.size() - 1)
			{
				std::swap(primitives[selected_primitive], primitives[selected_primitive + 1]);
				selected_primitive++;
			}
		}
		ImGui::PopID();
		ImGui::EndGroup();

		const auto camera_fovy = 45.f;
		const auto camera_zNear = 1.f;
		const auto camera_zFar = 1000.f;

		ImGui::SameLine();
		ImGui::BeginGroup();
		if (ImGui::Button("Zoom To Content"))
		{
			AABB bounds;
			bounds.reset();
			for (auto& p : points)
				bounds.expand(p.first);
			camera_pos = fit_camera_to_object(camera_rot, camera_fovy, camera_zNear, camera_zFar, bounds);
			camera_zoom = distance(camera_pos, bounds.center());
		}
		const vec2 origin = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("viewport", vec2(ImGui::GetContentRegionAvail()) - 2.f);
		auto p0 = ImGui::GetItemRectMin();
		auto p1 = ImGui::GetItemRectMax();
		auto sz = vec2(p1 - p0);

		auto camera_target = [&]() {
			return camera_pos - camera_rot[2] * camera_zoom;
		};

		if (ImGui::IsItemHovered())
		{
			auto& io = ImGui::GetIO();
			if (auto disp = (vec2)io.MouseDelta; disp.x != 0.f || disp.y != 0.f)
			{
				disp /= sz;
				if (!io.KeyAlt)
				{
					if (io.MouseDown[ImGuiMouseButton_Middle])
					{
						camera_pos += (-camera_rot[0] * disp.x +
							camera_rot[1] * disp.y) * camera_zoom;
					}
					else if (io.MouseDown[ImGuiMouseButton_Right])
					{
						disp *= -180.f;
						disp = radians(disp);
						auto qut = angleAxis(disp.x, vec3(0.f, 1.f, 0.f)) * camera_qut;
						qut = angleAxis(disp.y, qut * vec3(1.f, 0.f, 0.f)) * qut;
						camera_qut = qut;
						camera_rot = mat3(camera_qut);
					}
				}
				else
				{
					if (io.MouseDown[ImGuiMouseButton_Left])
					{
						disp *= -180.f;
						disp = radians(disp);
						auto qut = angleAxis(disp.x, vec3(0.f, 1.f, 0.f)) * camera_qut;
						qut = angleAxis(disp.y, qut * vec3(1.f, 0.f, 0.f)) * qut;
						camera_pos = camera_target() + (qut * vec3(0.f, 0.f, 1.f)) * camera_zoom;
						camera_qut = qut;
						camera_rot = mat3(camera_qut);
					}
				}
			}
			if (auto scroll = io.MouseWheel; scroll != 0.f)
			{
				auto tar = camera_target();
				if (scroll < 0.f)
					camera_zoom = camera_zoom * 1.1f + 0.5f;
				else
					camera_zoom = max(0.f, camera_zoom / 1.1f - 0.5f);
				camera_pos = tar + camera_rot[2] * camera_zoom;
			}
		}

		auto view_matrix = translate(mat4(1.f), camera_pos);
		view_matrix = view_matrix * mat4(camera_rot);
		view_matrix = inverse(view_matrix);
		auto project_matrix = perspective(camera_fovy, sz.x / sz.y, camera_zNear, camera_zFar);
		auto mvp_matrix = project_matrix * view_matrix;
		auto get_projected = [&](const vec3& p) {
			auto projected = mvp_matrix * vec4(p, 1.f);
			projected /= projected.w;
			projected.xy = projected.xy * 0.5f + 0.5f;
			projected.y = 1.f - projected.y;
			return projected.xy() * sz;
		};

		auto dl = ImGui::GetWindowDrawList();
		dl->AddRect(p0, p1, ImColor(1.f, 1.f, 1.f));
		for (auto& p : points)
		{
			auto pos = origin + get_projected(p.first);
			dl->AddCircleFilled(pos, 2.f, ImColor(1.f, 1.f, 1.f), 8);
			dl->AddText(pos + vec2(0.f, 4.f), ImColor(1.f, 1.f, 1.f), p.second.empty() ? str(p.first).c_str() : p.second.c_str());
		}
		for (auto& p : primitives)
		{
			switch (p.first)
			{
			case TypeLines:
			{
				if (p.second.size() < 2)
					continue;

				std::vector<vec2> draw_points;
				for (auto i = 0; i < p.second.size(); i++)
					draw_points.push_back(get_projected(points[p.second[i]].first));
				for (auto i = 0; i < draw_points.size() - 1; i++)
					dl->AddLine(origin + draw_points[i], origin + draw_points[i + 1], ImColor(1.f, 1.f, 1.f));
			}

				break;
			case TypePolygon:
			{
				if (p.second.size() < 3)
					continue;
				std::vector<vec2> draw_points;
				for (auto i = 0; i < p.second.size(); i++)
				{
					auto next = i + 1;
					if (next >= p.second.size()) next = 0;

					draw_points.push_back(get_projected(points[p.second[i]].first));
					draw_points.push_back(get_projected(points[p.second[next]].first));
				}

				bool back_face = true;
				for (auto i = 0; i < draw_points.size() - 2; i += 2)
				{
					if (cross(vec3(draw_points[i + 1] - draw_points[i + 0], 0.f), vec3(draw_points[i + 3] - draw_points[i + 2], 0.f)).z < 0.f)
					{
						back_face = false;
						break;
					}
				}

				for (auto i = 0; i < draw_points.size(); i += 2)
					dl->AddLine(origin + draw_points[i], origin + draw_points[i + 1], back_face ? ImColor(0.8f, 0.5f, 0.5f) : ImColor(1.f, 1.f, 1.f));
			}
				break;
			}
		}
		ImGui::EndGroup();

		ImGui::End();

		render_frames++;
	}
};

static App app;

int main(int argc, char** args)
{
	app.create("Primitive Debugger", uvec2(800, 600), WindowFrame | WindowResizable, true, true);
	app.run();

	return 0;
}
