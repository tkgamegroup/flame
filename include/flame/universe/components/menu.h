#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cMenu : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cMenu";
		inline static auto type_hash = ch(type_name);

		cMenu() :
			Component(type_name, type_hash)
		{
		}

//		enum Mode
//		{
//			ModeMenubar,
//			ModeMain,
//			ModeSub,
//			ModeContext
//		};
//
//		Mode mode;

		virtual void set_items(Entity* e) = 0;

		FLAME_UNIVERSE_EXPORTS static cMenu* create();
	};
//
//	struct cMenuItems : Component
//	{
//		cMenu* menu;
//
//		cMenuItems() :
//			Component("cMenuItems")
//		{
//		}
//
//		FLAME_UNIVERSE_EXPORTS static cMenuItems* create();
//	};
//
//	struct cMenuItem : Component
//	{
//		cEventReceiver* event_receiver;
//
//		cMenuItem() :
//			Component("cMenuItem")
//		{
//		}
//
//		FLAME_UNIVERSE_EXPORTS static cMenuItem* create();
//	};
//
//	FLAME_UNIVERSE_EXPORTS void close_menu(Entity* menu);
}
