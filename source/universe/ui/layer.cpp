#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/ui/layer.h>

namespace flame
{
	namespace ui
	{
		Entity* get_top_layer(Entity* parent)
		{
			if (parent->child_count() == 0)
				return nullptr;
			auto l = parent->child(parent->child_count() - 1);
			if (!l->dying_ && (l->name_hash() == FLAME_CHASH("layer") || l->name_hash() == FLAME_CHASH("layer_mmto")))
				return l;
			return nullptr;
		}

		Entity* add_layer(Entity* parent, bool penetrable, bool close_when_clicked, bool menu_move_to_open, const Vec4c& col, bool size_fit_parent)
		{
			assert(!get_top_layer(parent));

			auto l = Entity::create();
			l->set_name(!menu_move_to_open ? "layer" : "layer_mmto");
			parent->add_child(l);

			auto e_c_element = parent->get_component(cElement);
			assert(e_c_element);

			auto c_element = cElement::create();
			c_element->size_ = e_c_element->size_;
			c_element->color_ = col;
			l->add_component(c_element);

			auto c_event_receiver = cEventReceiver::create();
			c_event_receiver->penetrable = penetrable;
			if (close_when_clicked)
			{
				c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto e = *(Entity**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left && get_top_layer(e)->created_frame_ != looper().frame)
						remove_top_layer(e);
				}, new_mail_p(parent));
			}
			l->add_component(c_event_receiver);

			if (size_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy_ = SizeFitParent;
				c_aligner->height_policy_ = SizeFitParent;
				l->add_component(c_aligner);
			}

			return l;
		}

		void remove_top_layer(Entity* parent, bool take)
		{
			auto l = get_top_layer(parent);

			for (auto i = 0; i < l->child_count(); i++)
			{
				auto e = l->child(i);
				auto menu = e->get_component(cMenu);
				if (menu)
				{
					if (menu->button)
						menu->button->close();
					else
						close_menu(e);
				}
			}

			if (take)
				l->remove_child((Entity*)FLAME_INVALID_POINTER, false);
			l->dying_ = true;
			looper().add_event([](void* c) {
				auto t = *(Entity**)c;
				t->parent()->remove_child(t, false);
			}, new_mail_p(l));
		}
	}
}
