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
		ImageViewRGB,
		ImageViewRRR,
		ImageViewGGG,
		ImageViewBBB,
		ImageViewAAA
	};

	enum ImageViewSampler
	{
		ImageViewLinear,
		ImageViewNearest
	};

	struct ImageViewType
	{
		ImageViewSwizzle swizzle : 8 = ImageViewRGBA;
		ImageViewSampler sampler : 8 = ImageViewLinear;
		uint level : 8 = 0;
		uint layer : 8 = 0;
	};

	inline bool operator==(const ImageViewType& a, const ImageViewType& b)
	{
		return a.swizzle == b.swizzle && a.sampler == b.sampler && a.level == b.level && a.layer == b.layer;
	}

	void BeginRotation(float angle);
	void EndRotation();

	bool ToolButton(const char* label, bool selected = false, float rotate = 0.f);

	ImageViewType GetCurrentImageViewType();
	void PushImageViewType(ImageViewType type);
	void PopImageViewType();
}
