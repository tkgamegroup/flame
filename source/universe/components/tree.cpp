#include <flame/universe/default_style.h>
#include <flame/universe/utils.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/tree.h>

namespace flame
{
	struct cTreeNodePrivate : cTreeNode
	{
		void* mouse_listener;

		cTreeNodePrivate()
		{
			text = nullptr;
			event_receiver = nullptr;
			style = nullptr;
			combobox = nullptr;

			unselected_color_normal = default_style.frame_color_normal;
			unselected_color_hovering = default_style.frame_color_hovering;
			unselected_color_active = default_style.frame_color_active;
			selected_color_normal = default_style.header_color_normal;
			selected_color_hovering = default_style.header_color_hovering;
			selected_color_active = default_style.header_color_active;
		}

		void on_added()
		{
			text = (cText*)(entity->find_component(cH("Text")));
			assert(text);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			style = (cStyleBgCol*)(entity->find_component(cH("StyleBgCol")));

			if (style)
			{
				unselected_color_normal = style->col_normal;
				unselected_color_hovering = style->col_hovering;
				unselected_color_active = style->col_active;
			}

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cComboboxItemPrivate**)c;
					auto combobox = thiz->combobox;
					combobox->select = thiz->entity;
					combobox->text->set_text(thiz->text->text());
					destroy_topmost();
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (style)
			{
				if (combobox->select == entity)
				{
					style->col_normal = selected_color_normal;
					style->col_hovering = selected_color_hovering;
					style->col_active = selected_color_active;
				}
				else
				{
					style->col_normal = unselected_color_normal;
					style->col_hovering = unselected_color_hovering;
					style->col_active = unselected_color_active;
				}
			}
		}
	};

	cTreeNode::~cTreeNode()
	{
		((cTreeNodePrivate*)this)->~cComboboxItemPrivate();
	}

	void cTreeNode::on_added()
	{
		((cTreeNodePrivate*)this)->on_added();
	}

	void cTreeNode::update()
	{
		((cTreeNodePrivate*)this)->update();
	}

	cTreeNode* cTreeNode::create()
	{
		return new cTreeNodePrivate();
	}

	struct cTreePrivate : cTree
	{
		cTreePrivate()
		{
		}

		~cTreePrivate()
		{
		}

		void on_added()
		{
			text = (cText*)(entity->find_component(cH("Text")));
			assert(text && !text->auto_size);
			menu_button = (cMenuButton*)(entity->find_component(cH("MenuButton")));
			assert(menu_button);

			auto menu = menu_button->menu;
			for (auto i = 0; i < menu->child_count(); i++)
				((cComboboxItem*)(menu->child(i)->find_component(cH("ComboboxItem"))))->combobox = this;
		}
	};

	cTree::~cTree()
	{
		((cTreePrivate*)this)->~cTreePrivate();
	}

	void cTree::on_added()
	{
		((cTreePrivate*)this)->on_added();
	}

	void cTree::update()
	{
	}

	cTree* cTree::create()
	{
		return new cTreePrivate;
	}
}
