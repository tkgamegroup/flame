#pragma once

#include "foundation/foundation.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{
	enum ImageViewType
	{
		ImageViewRGBA,
		ImageViewR,
		ImageViewG,
		ImageViewB,
		ImageViewA,
		ImageViewRGB
	};

	void BeginRotation(float angle);
	void EndRotation();

	bool ToolButton(const char* label, bool selected = false, float rotate = 0.f);

	ImageViewType GetCurrentImageViewType();
	void PushImageViewType(ImageViewType type);
	void PopImageViewType();
}
