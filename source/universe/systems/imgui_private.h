#pragma once

#include "imgui.h"

namespace flame
{
	struct sImguiPrivate : sImgui
	{
		void on_added() override;
		void update() override;
	};
}
