#pragma once

#include "../entity_private.h"
#include "drag_edit.h"

namespace flame
{
	struct dDragEditPrivate : dDragEdit
	{
		BasicType type = FloatingType;
		int i = 0;
		float f = 0.f;

		int drag_pos;
		int drag_i;
		float drag_f;
		bool dragging = false;

		bool changing = false;

		EntityPrivate* drag;
		cTextPrivate* drag_text;
		cReceiverPrivate* drag_receiver;
		EntityPrivate* edit;
		cTextPrivate* edit_text;
		cReceiverPrivate* edit_receiver;

		BasicType get_type() const override { return type; }
		void set_type(BasicType t) override;

		int get_int() const override { return i; }
		void set_int(int i) override;

		float get_float() const override { return f; }
		void set_float(float f) override;

		void on_load_finished() override;
	};
}
