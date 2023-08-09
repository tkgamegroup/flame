#pragma once

#include "foundation/foundation.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{
	enum ImageViewSwizzle
	{
		ImageViewRGBA,
		ImageViewR,
		ImageViewG,
		ImageViewB,
		ImageViewA,
		ImageViewRGB
	};

	enum ImageViewSampler
	{
		ImageViewLinear,
		ImageViewNearest
	};

	struct ImageViewType
	{
		ImageViewSwizzle swizzle : 16;
		ImageViewSampler sampler : 16;
	};

	inline bool operator==(const ImageViewType& a, const ImageViewType& b)
	{
		return a.swizzle == b.swizzle && a.sampler == b.sampler;
	}

	void BeginRotation(float angle);
	void EndRotation();

	bool ToolButton(const char* label, bool selected = false, float rotate = 0.f);

	ImageViewType GetCurrentImageViewType();
	void PushImageViewType(ImageViewType type);
	void PopImageViewType();
}
