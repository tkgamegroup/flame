//#pragma once
//
//#include <flame/universe/component.h>
//
//namespace flame
//{
//	struct cMenu : Component
//	{
//		enum Mode
//		{
//			ModeMenubar,
//			ModeMain,
//			ModeSub,
//			ModeContext
//		};
//
//		cElement* element;
//		cEventReceiver* event_receiver;
//
//		Entity* root;
//		Entity* items;
//		Mode mode;
//
//		bool opened;
//
//		cMenu() :
//			Component("cMenu")
//		{
//		}
//		
//		FLAME_UNIVERSE_EXPORTS static cMenu* create(Mode mode);
//	};
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
//}
