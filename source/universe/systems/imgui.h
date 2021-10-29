#pragma once

#include "../component.h"

#include <imgui.h>

namespace flame
{
	struct sImgui : Component
	{
		inline static auto type_name = "flame::sImgui";
		inline static auto type_hash = ch(type_name);

		sImgui() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static sImgui* create(void* parms = nullptr);
	};
}
