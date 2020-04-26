#pragma once

#include <flame/universe/world.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/data_keeper.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>

namespace flame
{
	struct Entity;

	namespace utils
	{
		inline void remove_layer(Entity* l)
		{
			l->set_name("");
			looper().add_event([](Capture& c) {
				auto l = c.data<Entity*>();
				l->parent()->remove_child(l);
			}, Capture().set_data(&l));
		}

		inline Entity* add_layer(Entity* parent, void* pass_gene = nullptr, bool modal = false, const Vec4c& col = Vec4c(0))
		{
			auto l = Entity::create();
			l->gene = l;
			l->set_name("layer");
			{
				auto ed = parent->world()->get_system(sEventDispatcher);
				auto focusing = ed->next_focusing;
				if (focusing == INVALID_POINTER)
					focusing = ed->focusing;
				if (focusing)
				{
					auto c_data_keeper = cDataKeeper::create();
					c_data_keeper->set_common_item(FLAME_CHASH("focusing"), common(focusing));
					l->add_component(c_data_keeper);
				}
			}
			l->on_removed_listeners.add([](Capture& c) {
				auto l = c.data<Entity*>();
				auto dp = l->get_component(cDataKeeper);
				if (dp)
				{
					auto er = (cEventReceiver*)dp->get_common_item(FLAME_CHASH("focusing")).p;
					l->world()->get_system(sEventDispatcher)->next_focusing = er;
				}
				return true;
			}, Capture().set_data(&l));

			{
				struct Capturing
				{
					Entity* p;
					Entity* l;
				}capture;
				capture.p = parent;
				capture.l = l;
				looper().add_event([](Capture& c) {
					auto& capture = c.data<Capturing>();
					capture.p->add_child(capture.l);
				}, Capture().set_data(&capture));
			}

			auto c_element = cElement::create();
			c_element->color = col;
			l->add_component(c_element);

			auto c_event_receiver = cEventReceiver::create();
			if (pass_gene)
			{
				c_event_receiver->pass_checkers.add([](Capture& c, cEventReceiver* er, bool* pass) {
					if (er->entity->gene == c.data<void*>())
						*pass = true;
					return true;
				}, Capture().set_data(&pass_gene));
			}
			if (!modal)
			{
				c_event_receiver->mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
						remove_layer(c.data<Entity*>());
					return true;
				}, Capture().set_data(&l));
			}
			l->add_component(c_event_receiver);

			auto c_aligner = cAligner::create();
			c_aligner->x_align_flags = AlignMinMax;
			c_aligner->y_align_flags = AlignMinMax;
			l->add_component(c_aligner);

			return l;
		}
	}
}
