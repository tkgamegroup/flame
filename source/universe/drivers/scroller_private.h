#pragma once

#include "../entity_private.h"
#include <flame/universe/drivers/scroller.h>

namespace flame
{
	struct cElementPrivate;
	struct cReceiverPrivate;

	struct dScrollerPrivate : dScroller
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

		void on_load_finished() override;
		bool on_child_added(Entity* e) override;

		void scroll(const vec2& v) override;
	};
}
