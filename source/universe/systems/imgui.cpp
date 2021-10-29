#include "imgui_private.h"

namespace flame
{
	sImgui* sImgui::create(void* parms)
	{
		return new sImguiPrivate();
	}
}
