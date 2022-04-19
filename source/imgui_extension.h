#pragma once

#include <imgui.h>

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

	ImageViewType GetCurrentImageViewType();
	void PushImageViewType(ImageViewType type);
	void PopImageViewType();
}
