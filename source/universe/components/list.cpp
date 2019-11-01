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
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (!selected)
			{
				if (background_style)
				{
					background_style->color_normal = unselected_color_normal;
					background_style->color_hovering = unselected_color_hovering;
					background_style->color_active = unselected_color_active;
					background_style->style();
				}
				if (text_style)
				{
					text_style->color_normal = unselected_text_color_normal;
					text_style->color_else = unselected_text_color_else;
					text_style->style();
				}
			}
			else
			{
				if (background_style)
				{
					background_style->color_normal = selected_color_normal;
					background_style->color_hovering = selected_color_hovering;
					background_style->color_active = selected_color_active;
					background_style->style();
				}
				if (text_style)
				{
					text_style->color_normal = selected_text_color_normal;
					text_style->color_else = selected_text_color_else;
					text_style->style();
				}
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = *(cListItemPrivate**)c;

					if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
					{
						if (thiz->list)
							thiz->list->set_selected(thiz->entity);
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == cH("StyleColor"))
			{
				background_style = (cStyleColor*)c;
				do_style(false);
			}
			else if (c->name_hash == cH("StyleTextColor"))
			{
				text_style = (cStyleTextColor*)c;
				do_style(false);
			}
		}

		Component* copy() override
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

	cListItem* cListItem::create()
	{
		return new cListItemPrivate();
	}

	struct cListPrivate : cList
	{
		bool select_air_when_clicked;
		void* mouse_listener;

		cListPrivate(bool _select_air_when_clicked)
		{
			event_receiver = nullptr;

			selected = nullptr;

			select_air_when_clicked = _select_air_when_clicked;
			mouse_listener = nullptr;
		}

		~cListPrivate()
		{
			if (!entity->dying_ && mouse_listener)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				if (select_air_when_clicked)
				{
					mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
						auto thiz = *(cListPrivate**)c;

						if (is_mouse_down(action, key, true) && key == Mouse_Left)
							thiz->set_selected(nullptr);
					}, new_mail_p(this));
				}
			}
		}

		void on_child_component_added(Component* c) override
		{
			if (c->name_hash == cH("ListItem"))
				((cListItem*)c)->list = this;
		}

		Component* copy() override
		{
			auto copy = new cListPrivate(select_air_when_clicked);
			return copy;
		}
	};

	void cList::set_selected(Entity* e, bool trigger_changed)
	{
		if (selected == e)
			return;
		if (selected)
		{
			auto listitem = (cListItemPrivate*)selected->get_component(ListItem);
			if (listitem)
				listitem->do_style(false);
		}
		if (e)
		{
			auto listitem = (cListItemPrivate*)e->get_component(ListItem);
			if (listitem)
				listitem->do_style(true);
		}
		selected = e;
		if (trigger_changed)
			data_changed(cH("selected"), nullptr);
	}

	cList* cList::create(bool select_air_when_clicked)
	{
		return new cListPrivate(select_air_when_clicked);
	}

	Entity* create_standard_list(bool size_fit_parent)
	{
		auto e_list = Entity::create();
		{
			e_list->add_component(cElement::create());

			e_list->add_component(cEventReceiver::create());

			if (size_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy_ = SizeFitParent;
				c_aligner->height_policy_ = SizeFitParent;
				e_list->add_component(c_aligner);
			}

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			e_list->add_component(c_layout);

			e_list->add_component(cList::create());
		}

		return e_list;
	}

	Entity* create_standard_listitem(graphics::FontAtlas* font_atlas, float font_size_scale, const std::wstring& text)
	{
		auto e_item = Entity::create();
		{
			e_item->add_component(cElement::create());

			auto c_text = cText::create(font_atlas);
			c_text->font_size_ = default_style.font_size * font_size_scale;
			c_text->set_text(text);
			e_item->add_component(c_text);

			e_item->add_component(cEventReceiver::create());

			e_item->add_component(cStyleColor::create(Vec4c(0), Vec4c(0), Vec4c(0)));

			e_item->add_component(cListItem::create());

			auto c_aligner = cAligner::create();
			c_aligner->width_policy_ = SizeFitParent;
			e_item->add_component(c_aligner);
		}

		return e_item;
	}
}
