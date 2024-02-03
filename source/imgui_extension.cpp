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

	bool ToolButton(const char* label, bool selected, float rotation)
	{
		if (selected)
			PushStyleColor(ImGuiCol_Button, GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (rotation != 0.f)
			BeginRotation(rotation);
		auto clicked = Button(label);
		if (selected)
			PopStyleColor();
		if (rotation != 0.f)
			EndRotation();
		return clicked;
	}

	void ToolButton(const char* label, bool* v, float rotation)
	{
		if (*v)
			PushStyleColor(ImGuiCol_Button, GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (rotation != 0.f)
			BeginRotation(rotation);
		auto clicked = Button(label);
		if (*v)
			PopStyleColor();
		if (rotation != 0.f)
			EndRotation();
		if (clicked)
			*v = !*v;
	}

	std::stack<ImageViewArguments> image_view_args_list;

	ImageViewArguments GetCurrentImageViewType()
	{
		if (image_view_args_list.empty())
			return {};
		return image_view_args_list.top();
	}

	static int cmd_beg;

	void PushImageViewType(ImageViewArguments args)
	{
		cmd_beg = GetWindowDrawList()->CmdBuffer.Size;
		image_view_args_list.push(args);
	}

	void PopImageViewType()
	{
		auto args = image_view_args_list.top();
		image_view_args_list.pop();
		auto& cmds = GetWindowDrawList()->CmdBuffer;
		for (auto i = cmd_beg; i < cmds.Size; i++)
			memcpy(&cmds[i].UserCallbackData, &args, sizeof(args));
	}
}
