#pragma once

#include <flame/serialize.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>

namespace flame
{
	struct Entity;

	namespace ui
	{
		inline Entity* get_top_layer(Entity* parent, bool check = false)
		{
			if (parent->child_count() == 0)
				return nullptr;
			auto l = parent->child(parent->child_count() - 1);
			if (l->dying_)
				return nullptr;
			if (check && !SUS::starts_with(l->name(), "layer_"))
				return nullptr;
			return l;
		}

		inline void remove_top_layer(Entity* parent, bool take = true)
		{
			auto l = get_top_layer(parent);

			if (take)
				l->remove_child((Entity*)INVALID_POINTER, false);
			l->dying_ = true;
			looper().add_event([](void* c) {
				auto l = *(Entity**)c;
				l->parent()->remove_child(l);
			}, new_mail_p(l));
		}

		inline Entity* add_layer(Entity* parent, const char* name_suffix /* layer_* */, Entity* pass = nullptr, bool modal = false, const Vec4c& col = Vec4c(0))
		{
			auto l = Entity::create();
			l->set_name((std::string("layer_") + name_suffix).c_str());
			parent->add_child(l);

			auto e_c_element = parent->get_component(cElement);
			assert(e_c_element);

			auto c_element = cElement::create();
			c_element->color_ = col;
			l->add_component(c_element);

			auto c_event_receiver = cEventReceiver::create();
			c_event_receiver->pass = pass;
			if (!modal)
			{
				c_event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					auto e = *(Entity**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto l = get_top_layer(e);
						if (l)
							remove_top_layer(e);
					}
				}, new_mail_p(parent));
			}
			l->add_component(c_event_receiver);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy_ = SizeFitParent;
			c_aligner->height_policy_ = SizeFitParent;
			l->add_component(c_aligner);

			return l;
		}
	}
}
