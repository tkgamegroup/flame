#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include "../universe_private.h"
#include "../entity_private.h"
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/tree.h>

namespace flame
{
	cTree* get_tree(Entity* e)
	{
		auto p = e->parent();
		if (!p)
			return nullptr;
		auto t = p->get_component(Tree);
		if (t)
			return t;
		return p->parent()->get_component(TreeNode)->tree;
	}

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
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void do_style(bool selected)
		{
			if (!selected)
			{
				if (style)
				{
					style->color_normal = unselected_color_normal;
					style->color_hovering = unselected_color_hovering;
					style->color_active = unselected_color_active;
					style->style();
				}
			}
			else
			{
				if (style)
				{
					style->color_normal = selected_color_normal;
					style->color_hovering = selected_color_hovering;
					style->color_active = selected_color_active;
					style->style();
				}
			}
		}

		void on_added() override
		{
			tree = get_tree(entity);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
					{
						auto thiz = *(cTreeLeafPrivate**)c;
						thiz->tree->set_selected(thiz->entity);
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == cH("StyleColor"))
			{
				style = (cStyleColor*)c;
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
			if (tree)
			{
				auto title = entity->child(0);
				title->get_component(TreeNodeTitle)->tree = tree;
				title->child(0)->get_component(TreeNodeArrow)->tree = tree;
			}
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

			unselected_color_normal = default_style.frame_color_normal;
			unselected_color_hovering = default_style.frame_color_hovering;
			unselected_color_active = default_style.frame_color_active;
			selected_color_normal = default_style.selected_color_normal;
			selected_color_hovering = default_style.selected_color_hovering;
			selected_color_active = default_style.selected_color_active;

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
				if (!selected)
				{
					style->color_normal = unselected_color_normal;
					style->color_hovering = unselected_color_hovering;
					style->color_active = unselected_color_active;
					style->style();
				}
				else
				{
					style->color_normal = selected_color_normal;
					style->color_hovering = selected_color_hovering;
					style->color_active = selected_color_active;
					style->style();
				}
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && (key == Mouse_Left || key == Mouse_Right))
					{
						auto thiz = *(cTreeNodeTitlePrivate**)c;
						thiz->tree->set_selected(thiz->entity->parent());
					}
				}, new_mail_p(this));
			}
			else if (c->name_hash == cH("StyleColor"))
			{
				style = (cStyleColor*)c;
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

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("Text"))
				text = (cText*)c;
			else if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						auto thiz = *(cTreeNodeArrowPrivate**)c;
						auto e = thiz->entity->parent()->parent()->child(1);
						e->set_visibility(!e->visibility_);
						thiz->text->set_text(e->visibility_ ? Icon_ANGLE_DOWN : Icon_CARET_RIGHT);
					}
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
			if (c->name_hash == cH("EventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
						(*(cTreePrivate**)c)->set_selected(nullptr);
				}, new_mail_p(this));
			}
		}
	};

	void cTree::set_selected(Entity* e, bool trigger_changed)
	{
		if (selected)
		{
			auto treeleaf = (cTreeLeafPrivate*)selected->get_component(TreeLeaf);
			if (treeleaf)
				treeleaf->do_style(false);
			else
			{
				auto treenodetitle = (cTreeNodeTitlePrivate*)selected->child(0)->get_component(TreeNodeTitle);
				if (treenodetitle)
					treenodetitle->do_style(false);
			}
		}
		if (e)
		{
			auto treeleaf = (cTreeLeafPrivate*)e->get_component(TreeLeaf);
			if (treeleaf)
				treeleaf->do_style(true);
			else
			{
				auto treenodetitle = (cTreeNodeTitlePrivate*)e->child(0)->get_component(TreeNodeTitle);
				if (treenodetitle)
					treenodetitle->do_style(true);
			}
		}
		selected = e;
		if (trigger_changed)
			data_changed(cH("selected"), nullptr);
	}

	cTree* cTree::create()
	{
		return new cTreePrivate;
	}

	Entity* create_standard_tree(bool size_fit_parent)
	{
		auto e_tree = Entity::create();
		{
			e_tree->add_component(cElement::create());

			e_tree->add_component(cEventReceiver::create());

			if (size_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy_ = SizeFitParent;
				c_aligner->height_policy_ = SizeFitParent;
				e_tree->add_component(c_aligner);
			}

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			c_layout->width_fit_children = !size_fit_parent;
			c_layout->height_fit_children = !size_fit_parent;
			e_tree->add_component(c_layout);

			e_tree->add_component(cTree::create());
		}

		return e_tree;
	}

	Entity* create_standard_tree_node(graphics::FontAtlas* font_atlas, const std::wstring& name)
	{
		auto e_tree_node = Entity::create();
		{
			e_tree_node->add_component(cElement::create());

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			e_tree_node->add_component(c_layout);

			e_tree_node->add_component(cTreeNode::create());
		}

		auto e_title = Entity::create();
		e_tree_node->add_child(e_title);
		{
			auto c_element = cElement::create();
			c_element->inner_padding_ = Vec4f(default_style.font_size + 4.f, 2.f, 4.f, 2.f);
			e_title->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->set_text(name);
			e_title->add_component(c_text);

			e_title->add_component(cEventReceiver::create());

			e_title->add_component(cStyleColor::create());

			e_title->add_component(cLayout::create(LayoutFree));

			auto c_title = cTreeNodeTitle::create();
			c_title->unselected_color_normal = Vec4c(0);
			e_title->add_component(c_title);

			auto e_arrow = Entity::create();
			e_title->add_child(e_arrow);
			{
				auto c_element = cElement::create();
				c_element->inner_padding_ = Vec4f(0.f, 2.f, 4.f, 2.f);
				e_arrow->add_component(c_element);

				auto c_text = cText::create(font_atlas);
				c_text->set_text(Icon_ANGLE_DOWN);
				e_arrow->add_component(c_text);

				e_arrow->add_component(cEventReceiver::create());

				e_arrow->add_component(cStyleTextColor::create(default_style.text_color_normal, default_style.text_color_else));

				e_arrow->add_component(cTreeNodeArrow::create());
			}
		}

		auto e_sub_tree = Entity::create();
		e_tree_node->add_child(e_sub_tree);
		{
			auto c_element = cElement::create();
			c_element->inner_padding_ = Vec4f(default_style.font_size * 0.5f, 0.f, 0.f, 0.f);
			e_sub_tree->add_component(c_element);

			auto c_layout = cLayout::create(LayoutVertical);
			c_layout->item_padding = 4.f;
			e_sub_tree->add_component(c_layout);
		}

		return e_tree_node;
	}

	Entity* create_standard_tree_leaf(graphics::FontAtlas* font_atlas, const std::wstring& name)
	{
		auto e_tree_leaf = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->inner_padding_ = Vec4f(default_style.font_size + 4.f, 2.f, 4.f, 2.f);
			e_tree_leaf->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->set_text(name);
			e_tree_leaf->add_component(c_text);

			e_tree_leaf->add_component(cEventReceiver::create());

			e_tree_leaf->add_component(cStyleColor::create());

			auto c_tree_leaf = cTreeLeaf::create();
			c_tree_leaf->unselected_color_normal = Vec4c(0);
			e_tree_leaf->add_component(c_tree_leaf);
		}

		return e_tree_leaf;
	}
}
