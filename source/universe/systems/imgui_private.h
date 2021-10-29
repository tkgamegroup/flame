#pragma once

#include "imgui.h"

namespace flame
{
	struct sImguiPrivate : sImgui
	{
		void update() override;
	};
}
