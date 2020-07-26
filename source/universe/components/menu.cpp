//#include <flame/graphics/font.h>
//#include <flame/universe/components/element.h>
//#include <flame/universe/components/text.h>
//#include <flame/universe/components/event_receiver.h>
//#include <flame/universe/components/style.h>
//#include <flame/universe/components/aligner.h>
//#include <flame/universe/components/layout.h>
//#include <flame/universe/components/menu.h>
//#include <flame/universe/ui/layer.h>
//
//namespace flame
//{
//	struct cMenuPrivate : cMenu
//	{
//		void* mouse_listener;
//
//		cMenuPrivate(Mode _mode)
//		{
//			element = nullptr;
//			event_receiver = nullptr;
//
//			root = nullptr;
//			items = f_new<Entity>();
//			{
//				items->add_component(cElement::create());
//				items->add_component(cLayout::create(LayoutVertical));
//				items->add_component(cMenuItems::create());
//			}
//			items->get_component(cMenuItems)->menu = this;
//			mode = _mode;
//
//			opened = false;
//
//			mouse_listener = nullptr;
//		}
//
//		~cMenuPrivate()
//		{
//			if (!entity->dying_)
//				event_receiver->mouse_listeners.remove(mouse_listener);
//			f_delete(items);
//		}
//
//		void close()
//		{
//			if (opened)
//			{
//				opened = false;
//
//				for (auto i : items->children)
//				{
//					auto menu = (cMenuPrivate*)i->get_component(cMenu);
//					if (menu)
//						menu->close();
//				}
//
//				((Entity*)items->gene)->remove_child(items, false);
//			}
//		}
//
//		void close_subs(Entity* p)
//		{
//			if (mode != ModeMain)
//			{
//				for (auto i : p->children)
//				{
//					auto menu = (cMenuPrivate*)i->get_component(cMenu);
//					if (menu)
//						menu->close();
//				}
//			}
//		}
//
//		bool can_open(KeyStateFlags action, MouseKey key)
//		{
//			if (mode == cMenu::ModeContext)
//			{
//				if (!event_receiver->is_focusing_and_not_normal() && is_mouse_down(action, key, true) && key == Mouse_Right)
//					return true;
//			}
//			else
//			{
//				if (is_mouse_down(action, key, true) && key == Mouse_Left)
//					return true;
//				else if (is_mouse_move(action, key))
//					return root->last_child(FLAME_CHASH("layer_menu"));
//			}
//			return false;
//		}
//
//		void open(const Vec2f& pos)
//		{
//			if (!opened)
//			{
//				if (mode == ModeContext)
//				{
//					auto layer = add_layer(root);
//					layer->name = "layer_menu";
//					layer->event_listeners.add([](Capture& c, EntityEvent e, void* t) {
//						if (e == EntityRemoved)
//							c.thiz<Entity>()->remove_children(0, -1, false);
//						return true;
//					}, Capture().set_thiz(layer));
//					auto items_element = items->get_component(cElement);
//					items_element->set_pos(Vec2f(pos));
//					items_element->set_scale(element->global_scale);
//					layer->add_child(items);
//
//					opened = true;
//				}
//				else
//				{
//					auto p = entity->parent;
//					if (p)
//						close_subs(p);
//
//					auto layer = root->last_child();
//					if (layer)
//					{
//						if (layer->name.h != FLAME_CHASH("layer_menu"))
//							layer = nullptr;
//					}
//					auto new_layer = !layer;
//
//					if (mode == ModeSub)
//						assert(layer);
//					else
//					{
//						if (mode == ModeMenubar)
//						{
//							if (!layer)
//								layer = add_layer(root, p);
//						}
//						else
//							layer = add_layer(root, entity);
//					}
//					if (new_layer)
//					{
//						layer->name = "layer_menu";
//						layer->event_listeners.add([](Capture& c, EntityEvent e, void* t) {
//							if (e == EntityRemoved)
//								c.thiz<Entity>()->remove_children(0, -1, false);
//							return true;
//						}, Capture().set_thiz(layer));
//					}
//
//					auto items_element = items->get_component(cElement);
//					switch (mode)
//					{
//					case ModeMenubar: case ModeMain:
//						items_element->set_pos(Vec2f(element->global_pos.x(), element->global_pos.y() + element->global_size.y()));
//						break;
//					case ModeSub:
//						items_element->set_pos(Vec2f(element->global_pos.x() + element->global_size.x(), element->global_pos.y()));
//						break;
//					}
//					items_element->set_scale(element->global_scale);
//					layer->add_child(items);
//
//					opened = true;
//				}
//			}
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityComponentAdded:
//				if (t == this)
//				{
//					element = entity->get_component(cElement);
//					event_receiver = entity->get_component(cEventReceiver);
//					assert(element);
//					assert(event_receiver);
//
//					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
//						auto thiz = c.thiz<cMenuPrivate>();
//						if (thiz->can_open(action, key))
//							thiz->open((Vec2f)pos);
//						return true;
//					}, Capture().set_thiz(this));
//				}
//				break;
//			}
//		}
//	};
//
//	cMenu* cMenu::create(cMenu::Mode mode)
//	{
//		return new cMenuPrivate(mode);
//	}
//
//	struct cMenuItemsPrivate : cMenuItems
//	{
//		cMenuItemsPrivate()
//		{
//			menu = nullptr;
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityRemoved:
//				if (menu)
//					menu->opened = false;
//				break;
//			}
//		}
//	};
//
//	cMenuItems* cMenuItems::create()
//	{
//		return new cMenuItemsPrivate();
//	}
//
//	struct cMenuItemPrivate : cMenuItem
//	{
//		void* mouse_listener;
//
//		cMenuItemPrivate()
//		{
//			mouse_listener = nullptr;
//		}
//
//		~cMenuItemPrivate()
//		{
//			if (!entity->dying_)
//				event_receiver->mouse_listeners.remove(mouse_listener);
//		}
//
//		void on_event(EntityEvent e, void* t) override
//		{
//			switch (e)
//			{
//			case EntityComponentAdded:
//				if (t == this)
//				{
//					event_receiver = entity->get_component(cEventReceiver);
//					assert(event_receiver);
//
//					mouse_listener = event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
//						auto thiz = c.thiz<cMenuItemPrivate>();
//						auto c_items = thiz->entity->parent->get_component(cMenuItems);
//						if (c_items)
//						{
//							auto menu = (cMenuPrivate*)c_items->menu;
//							auto layer = menu->root->last_child();
//							if (layer)
//							{
//								if (layer->name.h == FLAME_CHASH("layer_menu"))
//									menu->close_subs(menu->items);
//							}
//						}
//						return true;
//					}, Capture().set_thiz(this));
//				}
//				break;
//			}
//		}
//	};
//
//	cMenuItem* cMenuItem::create()
//	{
//		return new cMenuItemPrivate();
//	}
//}
