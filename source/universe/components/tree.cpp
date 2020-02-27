#include <flame/graphics/font.h>
#include "../universe_private.h"
#include "../entity_private.h"
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/ui/layer.h>
#include <flame/universe/ui/style_stack.h>

namespace flame
{
	cTree* get_tree(Entity* e)
	{
		auto p = e->parent();
		if (!p)
			return nullptr;
		auto t = p->get_component(cTree);
		if (t)
			return t;
		return p->parent()->get_component(cTreeNode)->tree;
	}

	struct cTreeLeafPrivate : cTreeLeaf
	{
		void* mouse_listener;

		cTreeLeafPrivate()
		{
			style = nullptr;
			event_receiver = nullptr;
			tree = nullptr;

			mouse_listener = nullptr;
		}

		~cTreeLeafPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (style)
			{
				style->level = selected ? 1 : 0;
				style->style();
			}
		}

		void on_added() override
		{
			tree = get_tree(entity);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
					{
						auto thiz = *(cTreeLeafPrivate**)c;
						thiz->tree->set_selected(thiz->entity);
					}
					return true;
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				style = (cStyleColor2*)c;
				style->level = 0;
				do_style(false);
			}
		}
	};

	cTreeLeaf* cTreeLeaf::create()
	{
		return new cTreeLeafPrivate();
	}

	struct cTreeNodePrivate : cTreeNode
	{
		cTreeNodePrivate()
		{
			tree = nullptr;
		}

		void on_added() override
		{
			tree = get_tree(entity);
		}
	};

	cTreeNode* cTreeNode::create()
	{
		return new cTreeNodePrivate();
	}

	struct cTreeNodeTitlePrivate : cTreeNodeTitle
	{
		void* mouse_listener;

		cTreeNodeTitlePrivate()
		{
			event_receiver = nullptr;
			style = nullptr;
			tree = nullptr;

			mouse_listener = nullptr;
		}

		~cTreeNodeTitlePrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (style)
			{
				style->level = selected ? 1 : 0;
				style->style();
			}
		}

		void on_added() override
		{
			tree = entity->parent()->get_component(cTreeNode)->tree;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
					{
						auto thiz = *(cTreeNodeTitlePrivate**)c;
						thiz->tree->set_selected(thiz->entity->parent());
					}
					return true;
				}, new_mail_p(this));
			}
			else if (c->name_hash == FLAME_CHASH("cStyleColor2"))
			{
				style = (cStyleColor2*)c;
				style->level = 0;
				((cTreeNodeTitlePrivate*)this)->do_style(false);
			}
		}
	};

	cTreeNodeTitle* cTreeNodeTitle::create()
	{
		return new cTreeNodeTitlePrivate();
	}

	struct cTreeNodeArrowPrivate : cTreeNodeArrow
	{
		void* mouse_listener;

		cTreeNodeArrowPrivate()
		{
			text = nullptr;
			event_receiver = nullptr;
			tree = nullptr;

			mouse_listener = nullptr;
		}

		~cTreeNodeArrowPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_added() override
		{
			tree = entity->parent()->parent()->get_component(cTreeNode)->tree;
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cText"))
				text = (cText*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto thiz = *(cTreeNodeArrowPrivate**)c;
						auto e = thiz->entity->parent()->parent()->child(1);
						e->set_visibility(!e->visibility_);
						thiz->text->set_text(e->visibility_ ? Icon_ANGLE_DOWN : Icon_CARET_RIGHT);
					}
					return true;
				}, new_mail_p(this));
			}
		}
	};

	cTreeNodeArrow* cTreeNodeArrow::create()
	{
		return new cTreeNodeArrowPrivate();
	}

	struct cTreePrivate : cTree
	{
		void* mouse_listener;

		cTreePrivate()
		{
			event_receiver = nullptr;

			selected = nullptr;
		}

		~cTreePrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
						(*(cTreePrivate**)c)->set_selected(nullptr);
					return true;
				}, new_mail_p(this));
			}
		}
	};

	void cTree::set_selected(Entity* e, bool trigger_changed)
	{
		if (selected)
		{
			auto treeleaf = (cTreeLeafPrivate*)selected->get_component(cTreeLeaf);
			if (treeleaf)
				treeleaf->do_style(false);
			else
			{
				auto treenodetitle = (cTreeNodeTitlePrivate*)selected->child(0)->get_component(cTreeNodeTitle);
				if (treenodetitle)
					treenodetitle->do_style(false);
			}
		}
		if (e)
		{
			auto treeleaf = (cTreeLeafPrivate*)e->get_component(cTreeLeaf);
			if (treeleaf)
				treeleaf->do_style(true);
			else
			{
				auto treenodetitle = (cTreeNodeTitlePrivate*)e->child(0)->get_component(cTreeNodeTitle);
				if (treenodetitle)
					treenodetitle->do_style(true);
			}
		}
		selected = e;
		if (trigger_changed)
			data_changed(FLAME_CHASH("selected"), nullptr);
	}

	cTree* cTree::create()
	{
		return new cTreePrivate;
	}
}
