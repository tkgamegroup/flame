//#pragma once
//
//#include <flame/universe/component.h>
//
//namespace flame
//{
//	namespace graphics
//	{
//		struct FontAtlas;
//	}
//
//	struct cElement;
//	struct cText;
//	struct cEventReceiver;
//	struct cStyleColor2;
//
//	struct cCombobox : Component
//	{
//		cText* text;
//		cEventReceiver* event_receiver;
//
//		int index;
//
//		cCombobox() :
//			Component("cCombobox")
//		{
//		}
//
//		FLAME_UNIVERSE_EXPORTS void set_index(int index);
//
//		FLAME_UNIVERSE_EXPORTS static cCombobox* create();
//	};
//
//	struct cComboboxItem : Component
//	{
//		cEventReceiver* event_receiver;
//		cStyleColor2* style;
//
//		int index;
//
//		cComboboxItem() :
//			Component("cComboboxItem")
//		{
//		}
//
//		FLAME_UNIVERSE_EXPORTS static cComboboxItem* create();
//	};
//}
