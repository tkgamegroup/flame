#pragma once

#include "scroller.h"

namespace flame
{
	struct cScrollerPrivate : cScroller
	{
		ScrollType type = ScrollVertical;
		float step = 1.f;

		cElementPrivate* element = nullptr;
		cReceiverPrivate* receiver = nullptr;

		EntityPrivate* track = nullptr;
		cElementPrivate* track_element = nullptr;

		EntityPrivate* thumb = nullptr;
		cElementPrivate* thumb_element = nullptr;
		cReceiverPrivate* thumb_receiver = nullptr;

		EntityPrivate* view = nullptr;
		cElementPrivate* view_element = nullptr;

		EntityPrivate* target = nullptr;
		cElementPrivate* target_element = nullptr;

		ScrollType get_type() const override { return type; }
		void set_type(ScrollType type) override;

		void scroll(const vec2& v) override;

		void on_load_finished() override;
		bool on_before_adding_child(EntityPtr e) override;
	};
}
