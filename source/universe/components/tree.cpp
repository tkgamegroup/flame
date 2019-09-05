#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/graphics/font.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/tree.h>

namespace flame
{
	struct cTreeLeafPrivate : cTreeLeaf
	{
		void* mouse_listener;

		cTreeLeafPrivate()
		{
			style = nullptr;
			event_receiver = nullptr;
			tree = nullptr;

			unselected_color_normal = default_style.frame_color_normal;
			unselected_color_hovering = default_style.frame_color_hovering;
			unselected_color_active = default_style.frame_color_active;
			selected_color_normal = default_style.selected_color_normal;
			selected_color_hovering = default_style.selected_color_hovering;
			selected_color_active = default_style.selected_color_active;

			mouse_listener = nullptr;
		}

		~cTreeLeafPrivate()
		{
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			if (event_receiver)
				event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			style = (cStyleBackgroundColor*)(entity->find_component(cH("StyleBackgroundColor")));
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
			auto p = entity->parent();
			while (p)
			{
				tree = (cTree*)p->find_component(cH("Tree"));
				if (tree)
					break;
				p = p->parent();
			}
			assert(tree);

			if (style)
			{
				unselected_color_normal = style->color_normal;
				unselected_color_hovering = style->color_hovering;
				unselected_color_active = style->color_active;
			}

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cTreeLeafPrivate**)c;
					thiz->tree->selected = thiz->entity;
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (style)
			{
				if (tree->selected == entity)
				{
					style->color_normal = selected_color_normal;
					style->color_hovering = selected_color_hovering;
					style->color_active = selected_color_active;
				}
				else
				{
					style->color_normal = unselected_color_normal;
					style->color_hovering = unselected_color_hovering;
					style->color_active = unselected_color_active;
				}
			}
		}
	};

	void cTreeLeaf::start()
	{
		((cTreeLeafPrivate*)this)->start();
	}

	void cTreeLeaf::update()
	{
		((cTreeLeafPrivate*)this)->update();
	}

	cTreeLeaf* cTreeLeaf::create()
	{
		return new cTreeLeafPrivate();
	}

	struct cTreeNodePrivate : cTreeNode
	{
		void* title_mouse_listener;
		void* arrow_mouse_listener;

		cTreeNodePrivate()
		{
			title_style = nullptr;
			title_event_receiver = nullptr;
			arrow_text = nullptr;
			arrow_event_receiver = nullptr;
			tree = nullptr;

			unselected_color_normal = default_style.frame_color_normal;
			unselected_color_hovering = default_style.frame_color_hovering;
			unselected_color_active = default_style.frame_color_active;
			selected_color_normal = default_style.selected_color_normal;
			selected_color_hovering = default_style.selected_color_hovering;
			selected_color_active = default_style.selected_color_active;

			title_mouse_listener = nullptr;
			arrow_mouse_listener = nullptr;
		}

		~cTreeNodePrivate()
		{
			if (entity->child_count() == 0)
				return;

			auto e_title = entity->child(0);

			title_event_receiver = (cEventReceiver*)(e_title->find_component(cH("EventReceiver")));
			if (title_event_receiver)
				title_event_receiver->remove_mouse_listener(title_mouse_listener);

			if (e_title->child_count() == 0)
				return;

			auto e_arrow = e_title->child(0);
			arrow_event_receiver = (cEventReceiver*)(e_arrow->find_component(cH("EventReceiver")));
			if (arrow_event_receiver)
				arrow_event_receiver->remove_mouse_listener(arrow_mouse_listener);
		}

		void start()
		{
			auto e_title = entity->child(0);
			auto e_arrow = e_title->child(0);

			title_style = (cStyleBackgroundColor*)(e_title->find_component(cH("StyleBackgroundColor")));
			title_event_receiver = (cEventReceiver*)(e_title->find_component(cH("EventReceiver")));
			assert(title_event_receiver);
			arrow_text = (cText*)(e_arrow->find_component(cH("Text")));
			assert(arrow_text);
			arrow_event_receiver = (cEventReceiver*)(e_arrow->find_component(cH("EventReceiver")));
			assert(arrow_event_receiver);
			auto p = entity->parent();
			while (p)
			{
				tree = (cTree*)p->find_component(cH("Tree"));
				if (tree)
					break;
				p = p->parent();
			}
			assert(tree);

			if (title_style)
			{
				unselected_color_normal = title_style->color_normal;
				unselected_color_hovering = title_style->color_hovering;
				unselected_color_active = title_style->color_active;
			}

			title_mouse_listener = title_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cTreeNodePrivate**)c;
					thiz->tree->selected = thiz->entity;
				}
			}, new_mail_p(this));

			arrow_mouse_listener = arrow_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cTreeNodePrivate**)c;
					auto e = thiz->entity->child(1);
					e->visible = !e->visible;
					thiz->arrow_text->set_text(e->visible ? Icon_ANGLE_DOWN : Icon_CARET_RIGHT);
				}
			}, new_mail_p(this));
		}

		void update()
		{
			if (title_style)
			{
				if (tree->selected == entity)
				{
					title_style->color_normal = selected_color_normal;
					title_style->color_hovering = selected_color_hovering;
					title_style->color_active = selected_color_active;
				}
				else
				{
					title_style->color_normal = unselected_color_normal;
					title_style->color_hovering = unselected_color_hovering;
					title_style->color_active = unselected_color_active;
				}
			}
		}
	};

	void cTreeNode::start()
	{
		((cTreeNodePrivate*)this)->start();
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
	};

	void cTree::start()
	{
	}

	void cTree::update()
	{
	}

	cTree* cTree::create()
	{
		return new cTreePrivate;
	}
}
