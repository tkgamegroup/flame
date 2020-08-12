#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cScroller : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cScroller";
		inline static auto type_hash = ch(type_name);

		cScroller() :
			Component(type_name, type_hash)
		{
		}

		virtual ScrollerType get_type() const = 0;
		virtual void set_type(ScrollerType type) = 0;

		FLAME_UNIVERSE_EXPORTS static cScroller* create();
	};

//	struct cScrollbarThumb : Component
//	{
//		cElement* element;
//		cEventReceiver* event_receiver;
//		cScrollbar* scrollbar;
//		cElement* parent_element;
//		cLayout* target_layout;
//
//		ScrollbarType type;
//		float step;
//
//		FLAME_UNIVERSE_EXPORTS void update(float v);
//
//		FLAME_UNIVERSE_EXPORTS static cScrollbarThumb* create(ScrollbarType type);
//	};
}
