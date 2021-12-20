#include "text_private.h"
#include "receiver_private.h"
#include "drag_edit_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cDragEditPrivate::set_type(BasicType t)
	{
		assert(t == IntegerType || t == FloatType);
		type = t;
	}

	void cDragEditPrivate::set_int(int _i)
	{
		assert(type == IntegerType);
		i = clamp(_i, (int)min_v, (int)max_v);
		if (dragging)
			drag_i = i;
		drag_text->set_text(to_wstring(i));
	}

	void cDragEditPrivate::set_float(float _f)
	{
		assert(type == FloatType);
		f = clamp(_f, min_v, max_v);
		if (dragging)
			drag_f = f;
		drag_text->set_text(to_wstring(f));
	}

	void cDragEditPrivate::on_load_finished()
	{
		drag = entity->find_child("drag");
		assert(drag);
		drag_text = drag->get_component_t<cTextPrivate>();
		assert(drag_text);
		drag_receiver = drag->get_component_t<cReceiverPrivate>();
		assert(drag_receiver);

		drag_receiver->add_mouse_left_down_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<cDragEditPrivate>();
			thiz->drag_pos = pos.x;
			thiz->drag_i = thiz->i;
			thiz->drag_f = thiz->f;
		}, Capture().set_thiz(this));

		drag_receiver->add_mouse_left_up_listener([](Capture& c, const ivec2& pos) {
			auto thiz = c.thiz<cDragEditPrivate>();
			if (!thiz->dragging && thiz->drag_receiver->is_active())
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
			auto thiz = c.thiz<cDragEditPrivate>();
			if (thiz->drag_receiver->is_active())
			{
				thiz->dragging = true;
				switch (thiz->type)
				{
				case IntegerType:
				{
					auto c = int((pos.x - thiz->drag_pos) * thiz->sp);
					if (c != 0)
					{
						auto v = clamp(thiz->drag_i + c, (int)thiz->min_v, (int)thiz->max_v);
						if (v != thiz->i)
						{
							thiz->i = v;
							thiz->drag_pos = pos.x;
							thiz->drag_i = v;
							thiz->drag_text->set_text(to_wstring(v));
							thiz->data_changed(thiz, S<"value"_h>);
						}
					}
				}
					break;
				case FloatType:
				{
					auto c = (pos.x - thiz->drag_pos) * thiz->sp;
					if (c != 0.f)
					{
						auto v = clamp(thiz->drag_f + c, thiz->min_v, thiz->max_v);
						if (v != thiz->f)
						{
							thiz->f = thiz->drag_f + c;
							thiz->drag_pos = pos.x;
							thiz->drag_f = v;
							thiz->drag_text->set_text(to_wstring(v));
							thiz->data_changed(thiz, S<"value"_h>);
						}
					}
				}
					break;
				}
			}
			else
				thiz->dragging = false;
		}, Capture().set_thiz(this));

		edit = entity->find_child("edit");
		assert(edit);
		edit_text = edit->get_component_t<cTextPrivate>();
		assert(edit_text);
		edit_receiver = edit->get_component_t<cReceiverPrivate>();
		assert(edit_receiver);

		edit->add_message_listener([](Capture& c, uint msg, void* parm1, void* parm2) {
			if (msg == S<"state_changed"_h>)
			{
				auto thiz = c.thiz<cDragEditPrivate>();
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
				auto thiz = c.thiz<cDragEditPrivate>();
				if (thiz->changing)
					return;

				switch (thiz->type)
				{
				case IntegerType:
					thiz->i = sto<int>(thiz->edit_text->text);
					thiz->drag_text->set_text(to_wstring(thiz->i));
					break;
				case FloatType:
					thiz->f = sto<float>(thiz->edit_text->text);
					thiz->drag_text->set_text(to_wstring(thiz->f));
					break;
				}

				thiz->data_changed(thiz, S<"value"_h>);
			}
		}, Capture().set_thiz(this), edit_text);
	}

	cDragEdit* cDragEdit::create(void* parms)
	{
		return new cDragEditPrivate();
	}
}
