#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring string;

		//sLayoutManagement* management;

		//cElement* element;

		//void* draw_cmd;
		//bool pending_sizing;

		//cTextPrivate();
		//cTextPrivate::~cTextPrivate();
		//void draw(graphics::Canvas* canvas);
		//void auto_set_size();
		//void on_event(EntityEvent e, void* t) override;
	};
}
