#include <flame/universe/components/scroller.h>

namespace flame
{
	struct cEventReceiverPrivate;

	struct cScrollerPrivate : cScroller // R ~ on_*
	{
		ScrollerType type = ScrollerVertical;

		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_scroll_listener = nullptr;

		ScrollerType get_type() const override { return type; }
		void set_type(ScrollerType t) override { type = t; }

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}
