#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/list.h>
#include <flame/universe/ui/style_stack.h>

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

			mouse_listener = nullptr;
		}

		~cListItemPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (background_style)
			{
				background_style->level = selected ? 1 : 0;
				background_style->style();
			}
			if (text_style)
			{
				text_style->level = selected ? 1 : 0;
				text_style->style();
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
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
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				background_style = (cStyleColor2*)c;
				background_style->level = 0;
				do_style(false);
			}
			else if (c->name_hash == FLAME_CHASH("cStyleTextColor2"))
			{
				text_style = (cStyleTextColor2*)c;
				text_style->level = 0;
				do_style(false);
			}
		}

		Component* copy() override
		{
			return new cListItemPrivate;
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
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
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
			if (c->name_hash == FLAME_CHASH("cListItem"))
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
			auto listitem = (cListItemPrivate*)selected->get_component(cListItem);
			if (listitem)
				listitem->do_style(false);
		}
		if (e)
		{
			auto listitem = (cListItemPrivate*)e->get_component(cListItem);
			if (listitem)
				listitem->do_style(true);
		}
		selected = e;
		if (trigger_changed)
			data_changed(FLAME_CHASH("selected"), nullptr);
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

	Entity* create_standard_listitem(graphics::FontAtlas* font_atlas, float font_size_scale, const wchar_t* text)
	{
		auto e_item = Entity::create();
		{
			e_item->add_component(cElement::create());

			auto c_text = cText::create(font_atlas);
			c_text->font_size_ = ui::style(ui::FontSize).u()[0] * font_size_scale;
			c_text->set_text(text);
			e_item->add_component(c_text);

			e_item->add_component(cEventReceiver::create());

			auto c_style = cStyleColor2::create();
			c_style->color_normal[0] = ui::style(ui::FrameColorNormal).c();
			c_style->color_hovering[0] = ui::style(ui::FrameColorHovering).c();
			c_style->color_active[0] = ui::style(ui::FrameColorActive).c();
			c_style->color_normal[1] = ui::style(ui::SelectedColorNormal).c();
			c_style->color_hovering[1] = ui::style(ui::SelectedColorHovering).c();
			c_style->color_active[1] = ui::style(ui::SelectedColorActive).c();
			e_item->add_component(c_style);

			e_item->add_component(cListItem::create());

			auto c_aligner = cAligner::create();
			c_aligner->width_policy_ = SizeFitParent;
			e_item->add_component(c_aligner);
		}

		return e_item;
	}
}
