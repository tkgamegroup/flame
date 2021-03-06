#include "../components/text_private.h"
#include "../components/receiver_private.h"
#include "edit_private.h"
#include "drag_edit_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void dDragEditPrivate::set_type(BasicType t)
	{
		fassert(t == IntegerType || t == FloatingType);
		type = t;
	}

	void dDragEditPrivate::set_int(int _i)
	{
		fassert(type == IntegerType);
		i = _i;
		if (dragging)
			drag_i = i;
		drag_text->set_text(to_wstring(i));
	}

	void dDragEditPrivate::set_float(float _f)
	{
		fassert(type == FloatingType);
		f = _f;
		if (dragging)
			drag_f = f;
		drag_text->set_text(to_wstring(f));
	}

	void dDragEditPrivate::on_load_finished()
	{
		drag = entity->find_child("drag");
		fassert(drag);
		drag_text = drag->get_component_t<cTextPrivate>();
		fassert(drag_text);
		drag_receiver = drag->get_component_t<cReceiverPrivate>();
		fassert(drag_receiver);

		drag_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dDragEditPrivate>();
			thiz->drag_pos = pos.x;
			thiz->drag_i = thiz->i;
			thiz->drag_f = thiz->f;
		}, Capture().set_thiz(this));

		drag_receiver->add_mouse_left_up_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<dDragEditPrivate>();
			if (!thiz->dragging)
			{
				thiz->drag->set_visible(false);
				thiz->edit->set_visible(true);
				thiz->changing = true;
				thiz->edit_text->set_text(thiz->drag_text->text);
				thiz->changing = false;
				c.current<cReceiverPrivate>()->dispatcher->next_focusing = thiz->edit_receiver;
			}
			thiz->dragging = false;
		}, Capture().set_thiz(this));

		drag_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
			auto thiz = c.thiz<dDragEditPrivate>();
			if (thiz->drag_receiver->is_active())
			{
				thiz->dragging = true;
				switch (thiz->type)
				{
				case IntegerType:
				{
					auto c = (int)(pos.x - thiz->drag_pos) * 0.1f;
					if (c != 0)
					{
						thiz->i = thiz->drag_i + c;
						thiz->drag_pos = pos.x;
						thiz->drag_i = thiz->i;
						thiz->drag_text->set_text(to_wstring(thiz->i));
						thiz->entity->driver_data_changed(thiz, S<"value"_h>);
					}
				}
					break;
				case FloatingType:
				{
					auto c = (pos.x - thiz->drag_pos) * 0.05f;
					thiz->f = thiz->drag_f + c;
					thiz->drag_pos = pos.x;
					thiz->drag_f = thiz->f;
					thiz->drag_text->set_text(to_wstring(thiz->f));
					thiz->entity->driver_data_changed(thiz, S<"value"_h>);
				}
					break;
				}
			}
			else
				thiz->dragging = false;
		}, Capture().set_thiz(this));

		edit = entity->find_child("edit");
		fassert(edit);
		edit_text = edit->get_component_t<cTextPrivate>();
		fassert(edit_text);
		edit_receiver = edit->get_component_t<cReceiverPrivate>();
		fassert(edit_receiver);

		edit->add_message_listener([](Capture& c, uint msg, void* parm1, void* parm2) {
			if (msg == S<"state_changed"_h>)
			{
				auto thiz = c.thiz<dDragEditPrivate>();
				auto state = (int)parm1;
				auto last_state = (int)parm2;
				if ((state ^ last_state) & StateFocusing && !(state & StateFocusing))
				{
					thiz->drag->set_visible(true);
					thiz->edit->set_visible(false);
				}
			}
		}, Capture().set_thiz(this));

		edit->add_component_data_listener([](Capture& c, uint hash) {
			if (hash == S<"text"_h>)
			{
				auto thiz = c.thiz<dDragEditPrivate>();
				if (thiz->changing)
					return;

				switch (thiz->type)
				{
				case IntegerType:
					thiz->i = sto<int>(thiz->edit_text->text);
					thiz->drag_text->set_text(to_wstring(thiz->i));
					break;
				case FloatingType:
					thiz->f = sto<float>(thiz->edit_text->text);
					thiz->drag_text->set_text(to_wstring(thiz->f));
					break;
				}

				thiz->entity->driver_data_changed(thiz, S<"value"_h>);
			}
		}, Capture().set_thiz(this), edit_text);
	}

	dDragEdit* dDragEdit::create(void* parms)
	{
		return new dDragEditPrivate();
	}
}
