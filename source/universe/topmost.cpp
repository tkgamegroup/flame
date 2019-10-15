#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/menu.h>

namespace flame
{
	Entity* get_topmost(Entity* e)
	{
		if (e->child_count() == 0)
			return nullptr;
		auto t = e->child(e->child_count() - 1);
		if (t->name_hash() == cH("topmost") || t->name_hash() == cH("topmost_no_mousemove_to_open_menu"))
			return t;
		return nullptr;
	}

	Entity* create_topmost(Entity* e, bool penetrable, bool close_when_clicked, bool no_mousemove_to_open_menu, const Vec4c& col, bool size_fit_parent)
	{
		assert(!get_topmost(e));

		auto t = Entity::create();
		t->set_name(!no_mousemove_to_open_menu ? "topmost" : "topmost_no_mousemove_to_open_menu");
		e->add_child(t);

		{
			auto e_c_element = (cElement*)e->find_component(cH("Element"));
			assert(e_c_element);

			auto c_element = cElement::create();
			c_element->size = e_c_element->size;
			c_element->color = col;
			t->add_component(c_element);

			auto c_event_receiver = cEventReceiver::create();
			c_event_receiver->penetrable = penetrable;
			if (close_when_clicked)
			{
				c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto e = *(Entity**)c;
					if (is_mouse_down(action, key, true) && key == Mouse_Left && get_topmost(e)->created_frame != looper().frame)
						destroy_topmost(e);
				}, new_mail_p(e));
			}
			t->add_component(c_event_receiver);

			if (size_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				t->add_component(c_aligner);
			}
		}

		return t;
	}

	void destroy_topmost(Entity* e, bool take)
	{
		auto t = get_topmost(e);

		std::vector<Entity*> children;
		for (auto i = 0; i < t->child_count(); i++)
			children.push_back(t->child(i));
		for (auto e : children)
		{
			auto menu = (cMenu*)e->find_component(cH("Menu"));
			if (menu)
			{
				if (menu->popuped_by)
					menu->popuped_by->close();
				else
					close_menu(e);
			}
		}

		if (take)
			t->remove_all_children(false);
		looper().add_delay_event([](void* c) {
			auto t = *(Entity**)c;
			t->parent()->remove_child(t, false);
		}, new_mail_p(t));
	}
}
