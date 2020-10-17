#include "custom_drawing_private.h"

namespace flame
{
	void* cCustomDrawingPrivate::add_drawer(void (*callback)(Capture& c, graphics::Canvas* canvas), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		drawers.emplace_back(c);
		return c;
	}

	void cCustomDrawingPrivate::remove_drawer(void* lis)
	{
		std::erase_if(drawers, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	cCustomDrawing* cCustomDrawing::create()
	{
		return f_new<cCustomDrawingPrivate>();
	}
}
