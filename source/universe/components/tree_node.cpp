#include "../../graphics/font.h"
#include "../entity_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "tree_private.h"
#include "tree_node_private.h"

namespace flame
{
	void cTreeNodePrivate::set_title(std::wstring_view _title)
	{
		title = _title;
		if (load_finished)
			title_text->set_text(title.c_str());
	}

	void cTreeNodePrivate::notify_selected(bool v)
	{
		e_title->set_state(v ? (StateFlags)(e_title->state | StateSelected) : (StateFlags)(e_title->state & (~StateSelected)));
	}

	void cTreeNodePrivate::toggle_collapse()
	{
		items->set_visible(!items->visible);
		arrow_text->set_text(items->visible ? ICON_FA_CARET_DOWN : ICON_FA_CARET_RIGHT);
	}

	void cTreeNodePrivate::on_load_finished()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);

		e_title = entity->find_child("title");
		assert(e_title);
		title_text = e_title->get_component_t<cTextPrivate>();
		assert(title_text);
		auto title_receiver = e_title->get_component_t<cReceiverPrivate>();
		assert(title_receiver);

		title_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<cTreeNodePrivate>();
			thiz->tree->set_selected(thiz->entity);
		}, Capture().set_thiz(this));

		auto earrow = entity->find_child("arrow");
		assert(earrow);
		arrow_text = earrow->get_component_t<cTextPrivate>();
		assert(arrow_text);
		auto arrow_receiver = earrow->get_component_t<cReceiverPrivate>();
		assert(arrow_receiver);
		arrow_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			c.thiz<cTreeNodePrivate>()->toggle_collapse();
		}, Capture().set_thiz(this));

		items = entity->find_child("items");
		assert(items);
		items_element = items->get_component_i<cElementPrivate>(0);
		assert(items_element);
	}

	bool cTreeNodePrivate::on_before_adding_child(EntityPtr e)
	{
		if (load_finished)
		{
			if (first_add)
			{
				first_add = false;
				if (element->alignx == AlignMinMax)
				{
					items_element->set_auto_width(false);
					entity->add_component_data_listener([](Capture& c, uint hash) {
						if (hash == S<"width"_h>)
						{
							auto thiz = c.thiz<cTreeNodePrivate>();
							thiz->items_element->set_width(thiz->element->size.x);
						}
					}, Capture().set_thiz(this), element);
				}
			}
			populate_tree(tree, e);
			items->add_child(e);
			return true;
		}
		return false;
	}

	cTreeNode* cTreeNode::create(void* parms)
	{
		return new cTreeNodePrivate();
	}
}
