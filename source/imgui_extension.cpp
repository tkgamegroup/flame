#include "imgui_extension.h"

namespace ImGui
{
	static int vtx_beg;
	static float rotate_angle;

	void BeginRotation(float angle)
	{
		vtx_beg = GetWindowDrawList()->VtxBuffer.Size;
		rotate_angle = angle;
	}

	void EndRotation()
	{
		auto& vtxs = GetWindowDrawList()->VtxBuffer;

		glm::vec2 c;
		{
			auto lower = glm::vec2(+1000.f);
			auto upper = glm::vec2(-1000.f);
			for (auto i = vtx_beg; i < vtxs.Size; i++)
			{
				auto p = glm::vec2(vtxs[i].pos);
				lower = glm::min(lower, p);
				upper = glm::max(upper, p);
			}
			c = (lower + upper) * 0.5f;
		}

		auto rad = glm::radians(rotate_angle);
		auto sin_a = glm::sin(rad);
		auto cos_a = glm::cos(rad);
		for (auto i = vtx_beg; i < vtxs.Size; i++)
		{
			auto& p = vtxs[i].pos;
			p.x -= c.x; p.y -= c.y;
			p = ImVec2(cos_a * p.x - sin_a * p.y + c.x, sin_a * p.x + cos_a * p.y + c.y);
		}
	}

	bool ToolButton(const char* label, bool selected, float rotate)
	{
		if (selected)
			PushStyleColor(ImGuiCol_Button, GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (rotate != 0.f)
			BeginRotation(rotate);
		auto clicked = Button(label);
		if (selected)
			PopStyleColor();
		if (rotate != 0.f)
			EndRotation();
		return clicked;
	}

	std::stack<ImageViewType> image_view_types;

	ImageViewType GetCurrentImageViewType()
	{
		if (image_view_types.empty())
			return {};
		return image_view_types.top();
	}

	static int cmd_beg;

	void PushImageViewType(ImageViewType type)
	{
		cmd_beg = GetWindowDrawList()->CmdBuffer.Size;
		image_view_types.push(type);
	}

	void PopImageViewType()
	{
		auto type = image_view_types.top();
		image_view_types.pop();
		auto& cmds = GetWindowDrawList()->CmdBuffer;
		for (auto i = cmd_beg; i < cmds.Size; i++)
			memcpy(&cmds[i].UserCallbackData, &type, sizeof(type));
	}
}
