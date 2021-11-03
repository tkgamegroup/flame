#pragma once

#include "imgui.h"

namespace flame
{
	struct cImguiPrivate : cImgui
	{
		std::unique_ptr<Closure<void(Capture&)>> callback;

		void on_draw(void (*draw)(Capture& c), const Capture& capture) override;

		void on_entered_world() override;
		void on_left_world() override;
	};
}
