#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
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

			mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cTreeLeafPrivate**)c;
					thiz->tree->set_selected(thiz->entity);
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

			title_mouse_listener = title_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					auto thiz = *(cTreeNodePrivate**)c;
					thiz->tree->set_selected(thiz->entity);
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
		std::vector<std::unique_ptr<Closure<void(void* c, Entity * selected)>>> selected_changed_listeners;
	};

	void* cTree::add_selected_changed_listener(void (*listener)(void* c, Entity* selected), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, Entity * selected)>;
		c->function = listener;
		c->capture = capture;
		((cTreePrivate*)this)->selected_changed_listeners.emplace_back(c);
		return c;
	}

	void cTree::remove_selected_changed_listener(void* ret_by_add)
	{
		auto& listeners = ((cTreePrivate*)this)->selected_changed_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cTree::set_selected(Entity* e, bool trigger_changed)
	{
		selected = e;
		if (trigger_changed)
		{
			auto& listeners = ((cTreePrivate*)this)->selected_changed_listeners;
			for (auto& l : listeners)
				l->function(l->capture.p, selected);
		}
	}

	void cTree::update()
	{
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

			if (size_fit_parent)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				c_aligner->height_policy = SizeFitParent;
				e_tree->add_component(c_aligner);
			}

			auto c_layout = cLayout::create();
			c_layout->type = LayoutVertical;
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

			auto c_layout = cLayout::create();
			c_layout->type = LayoutVertical;
			c_layout->item_padding = 4.f;
			e_tree_node->add_component(c_layout);

			auto c_tree_node = cTreeNode::create();
			c_tree_node->unselected_color_normal = Vec4c(0);
			e_tree_node->add_component(c_tree_node);
		}

		auto e_title = Entity::create();
		e_tree_node->add_child(e_title);
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(4.f + font_atlas->pixel_height, 2.f, 4.f, 2.f);
			e_title->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->set_text(name);
			e_title->add_component(c_text);

			e_title->add_component(cEventReceiver::create());

			e_title->add_component(cStyleBackgroundColor::create());

			auto e_arrow = Entity::create();
			e_title->add_child(e_arrow);
			{
				auto c_element = cElement::create();
				c_element->inner_padding = Vec4f(0.f, 2.f, 4.f, 2.f);
				e_arrow->add_component(c_element);

				auto c_text = cText::create(font_atlas);
				c_text->set_text(Icon_ANGLE_DOWN);
				e_arrow->add_component(c_text);

				e_arrow->add_component(cEventReceiver::create());

				e_arrow->add_component(cStyleTextColor::create(default_style.text_color_normal, default_style.text_color_else));
			}
		}

		auto e_sub_tree = Entity::create();
		e_tree_node->add_child(e_sub_tree);
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(font_atlas->pixel_height * 0.5f, 0.f, 0.f, 0.f);
			e_sub_tree->add_component(c_element);

			auto c_layout = cLayout::create();
			c_layout->type = LayoutVertical;
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
			c_element->inner_padding = Vec4f(4.f + font_atlas->pixel_height, 2.f, 4.f, 2.f);
			e_tree_leaf->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->set_text(name);
			e_tree_leaf->add_component(c_text);

			e_tree_leaf->add_component(cEventReceiver::create());

			e_tree_leaf->add_component(cStyleBackgroundColor::create());

			auto c_tree_leaf = cTreeLeaf::create();
			c_tree_leaf->unselected_color_normal = Vec4c(0);
			e_tree_leaf->add_component(c_tree_leaf);
		}

		return e_tree_leaf;
	}
}
