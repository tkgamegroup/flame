#include "../world_private.h"
#include "element_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "menu_private.h"

namespace flame
{
	static cMenuPrivate* curr_menu = nullptr;


	void cMenuPrivate::on_gain_receiver()
	{
	}

	void cMenuPrivate::on_lost_receiver()
	{
		receiver->remove_mouse_left_down_listener(mouse_down_listener);
		receiver->remove_mouse_move_listener(mouse_move_listener);
	}

	//void cMenuPrivate::on_local_message(Message msg, void* p)
	//{
	//	switch (msg)
	//	{
	//	case MessageEnteredWorld:
	//		root = entity->world->root.get();
	//		root_receiver = root->get_component_t<cReceiverPrivate>();
	//		break;
	//	}
	//}

	//void cMenuPrivate::on_child_message(Message msg, void* p)
	//{
	//	switch (msg)
	//	{
	//	case MessageAdded:
	//		if (!items && ((EntityPrivate*)p)->name != "arrow")
	//			items = (EntityPrivate*)p;
	//		break;
	//	case MessageRemoved:
	//		if (items == p && !opened)
	//			items = nullptr;
	//		break;
	//	}
	//}

	cMenu* cMenu::create()
	{
		return f_new<cMenuPrivate>();
	}
}
