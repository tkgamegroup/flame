#include "../entity_private.h"
#include "text_private.h"
#include "receiver_private.h"
#include "tree_private.h"
#include "tree_leaf_private.h"

namespace flame
{
	void cTreeLeafPrivate::set_title(std::wstring_view _title)
	{
		title = _title;
		if (load_finished)
			title_text->set_text(title.c_str());
	}

	void cTreeLeafPrivate::notify_selected(bool v)
	{
		entity->set_state(v ? (StateFlags)(entity->state | StateSelected) : (StateFlags)(entity->state & (~StateSelected)));
	}

	void cTreeLeafPrivate::on_load_finished()
	{
		title_text = entity->get_component_t<cTextPrivate>();
		assert(title_text);
		auto receiver = entity->get_component_t<cReceiverPrivate>();
		assert(receiver);

		receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<cTreeLeafPrivate>();
			thiz->tree->set_selected(thiz->entity);
		}, Capture().set_thiz(this));
	}

	cTreeLeaf* cTreeLeaf::create(void* parms)
	{
		return new cTreeLeafPrivate();
	}
}
