#include "imgui_private.h"

namespace flame
{
	void cImguiPrivate::on_draw(void(*draw)(Capture& c), const Capture& capture)
	{
		callback.reset(new Closure(draw, capture));
	}

	cImgui* cImgui::create(void* parms)
	{
		return new cImguiPrivate();
	}
}
