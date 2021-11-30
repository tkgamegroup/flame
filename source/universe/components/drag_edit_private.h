#pragma once

#include "drag_edit.h"

namespace flame
{
	struct cDragEditPrivate : cDragEdit
	{
		BasicType type = FloatType;
		int i = 0;
		float f = 0.f;
		float min_v = -10000.f;
		float max_v = +10000.f;
		float sp = 0.1f;

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

		float get_min() const override { return min_v; }
		void set_min(float v) override { min_v = v; }
		float get_max() const override { return max_v; }
		void set_max(float v) override { max_v = v; }

		float get_speed() const override { return sp; }
		void set_speed(float v) override { sp = v; }

		void on_load_finished() override;
	};
}
