#include "imgui_extension.h"

namespace ImGui
{
	static int idx_beg;
	static float rotate_angle;

	void BeginRotation(float angle)
	{
		idx_beg = GetWindowDrawList()->VtxBuffer.Size;
		rotate_angle = angle;
	}

	void EndRotation()
	{
		auto& vtxs = GetWindowDrawList()->VtxBuffer;

		glm::vec2 c;
		{
			auto lower = glm::vec2(+1000.f);
			auto upper = glm::vec2(-1000.f);
			for (auto i = idx_beg; i < vtxs.Size; i++)
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
		for (auto i = idx_beg; i < vtxs.Size; i++)
		{
			auto& p = vtxs[i].pos;
			p.x -= c.x; p.y -= c.y;
			p = ImVec2(cos_a * p.x - sin_a * p.y + c.x, sin_a * p.x + cos_a * p.y + c.y);
		}
	}
}
