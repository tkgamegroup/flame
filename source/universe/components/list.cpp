#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/list.h>

namespace flame
{
	struct cListItemPrivate : cListItem
	{
		void* mouse_listener;

		cListItemPrivate()
		{
			event_receiver = nullptr;
			background_style = nullptr;
			text_style = nullptr;
			list = nullptr;

			unselected_color_normal = default_style.frame_color_normal;
			unselected_color_hovering = default_style.frame_color_hovering;
			unselected_color_active = default_style.frame_color_active;
			unselected_text_color_normal = default_style.text_color_normal;
			unselected_text_color_else = default_style.text_color_else;
			selected_color_normal = default_style.selected_color_normal;
			selected_color_hovering = default_style.selected_color_hovering;
			selected_color_active = default_style.selected_color_active;
			selected_text_color_normal = default_style.text_color_normal;
			selected_text_color_normal = default_style.text_color_else;

			mouse_listener = nullptr;
		}

		~cListItemPrivate()
		{
			if (!entity->dying)
				event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			background_style = (cStyleColor*)(entity->find_component(cH("StyleColor")));
			text_style = (cStyleTextColor*)(entity->find_component(cH("StyleTextColor")));
			list = (cList*)(entity->parent()->find_component(cH("List")));

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
				{
					auto thiz = *(cListItemPrivate**)c;
					if (thiz->list)
						thiz->list->set_selected(thiz->entity);
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (background_style)
			{
				if (list && list->selected == entity)
				{
					background_style->color_normal = selected_color_normal;
					background_style->color_hovering = selected_color_hovering;
					background_style->color_active = selected_color_active;
				}
				else
				{
					background_style->color_normal = unselected_color_normal;
					background_style->color_hovering = unselected_color_hovering;
					background_style->color_active = unselected_color_active;
				}
			}
			if (text_style)
			{
				if (list && list->selected == entity)
				{
					text_style->color_normal = selected_text_color_normal;
					text_style->color_else = selected_text_color_else;
				}
				else
				{
					text_style->color_normal = unselected_text_color_normal;
					text_style->color_else = unselected_text_color_else;
				}
			}
		}

		Component* copy()
		{
			auto copy = new cListItemPrivate();

			copy->unselected_color_normal = unselected_color_normal;
			copy->unselected_color_hovering = unselected_color_hovering;
			copy->unselected_color_active = unselected_color_active;
			copy->unselected_text_color_normal = unselected_text_color_normal;
			copy->unselected_text_color_else = unselected_text_color_else;
			copy->selected_color_normal = selected_color_normal;
			copy->selected_color_hovering = selected_color_hovering;
			copy->selected_color_active = selected_color_active;
			copy->selected_text_color_normal = selected_text_color_normal;
			copy->selected_text_color_else = selected_text_color_else;

			return copy;
		}
	};

	void cListItem::start()
	{
		((cListItemPrivate*)this)->start();
	}

	void cListItem::update()
	{
		((cListItemPrivate*)this)->update();
	}

	Component* cListItem::copy()
	{
		return ((cListItemPrivate*)this)->copy();
	}

	cListItem* cListItem::create()
	{
		return new cListItemPrivate();
	}

	struct cListPrivate : cList
	{
		std::vector<std::unique_ptr<Closure<void(void* c, Entity* selected)>>> selected_changed_listeners;
	};

	void* cList::add_selected_changed_listener(void (*listener)(void* c, Entity* selected), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, Entity * selected)>;
		c->function = listener;
		c->capture = capture;
		((cListPrivate*)this)->selected_changed_listeners.emplace_back(c);
		return c;
	}

	void cList::remove_selected_changed_listener(void* ret_by_add)
	{
		auto& listeners = ((cListPrivate*)this)->selected_changed_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cList::set_selected(Entity* e, bool trigger_changed)
	{
		selected = e;
		if (trigger_changed)
		{
			auto& listeners = ((cListPrivate*)this)->selected_changed_listeners;
			for (auto& l : listeners)
				l->function(l->capture.p, selected);
		}
	}

	void cList::update()
	{
	}

	Component* cList::copy()
	{
		return new cListPrivate();
	}

	cList* cList::create()
	{
		return new cListPrivate();
	}

	Entity* create_standard_list(bool size_fit_parent)
	{
		auto e_list = Entity::create();
		{
			e_list->add_component(cElement::create());

			auto c_event_receiver = cEventReceiver::create();
			e_list->add_component(c_event_receiver);

			if (size_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_list->add_component(c_aligner);
			}

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			e_list->add_component(c_layout);

			auto c_list = cList::create();
			e_list->add_component(c_list);

			c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				auto list = *(cList**)c;

				if (is_mouse_down(action, key, true) && key == Mouse_Left)
					list->selected = nullptr;
			}, new_mail_p(c_list));
		}

		return e_list;
	}

	Entity* create_standard_listitem(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text)
	{
		auto e_item = Entity::create();
		{
			e_item->add_component(cElement::create());

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			c_text->set_text(text);
			e_item->add_component(c_text);

			e_item->add_component(cEventReceiver::create());

			e_item->add_component(cStyleColor::create(Vec4c(0), Vec4c(0), Vec4c(0)));

			e_item->add_component(cListItem::create());

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeFitParent;
			e_item->add_component(c_aligner);
		}

		return e_item;
	}
}
