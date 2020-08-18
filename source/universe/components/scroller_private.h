#include <flame/universe/components/scroller.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;
	struct cLayoutPrivate;
	struct cScrollViewPrivate;

	struct cScrollerPrivate : cScroller // R ~ on_*
	{
		ScrollerType type = ScrollerVertical;

		float step = 1.f;

		cEventReceiverPrivate* event_receiver = nullptr; // R ref
		cElementPrivate* track_element = nullptr; // R ref place=track
		cEventReceiverPrivate* track_event_receiver = nullptr; // R ref place=track
		cElementPrivate* thumb_element = nullptr; // R ref place=thumb
		cEventReceiverPrivate* thumb_event_receiver = nullptr; // R ref place=thumb

		cElementPrivate* view_element = nullptr; // R ref place=view
		cLayoutPrivate* view_layout = nullptr; // R ref place=view
		cScrollViewPrivate* view = nullptr; // R ref place=view
		cElementPrivate* target_element = nullptr;

		void* mouse_scroll_listener = nullptr;
		void* track_element_listener = nullptr;
		void* thumb_mouse_listener = nullptr;

		ScrollerType get_type() const override { return type; }
		void set_type(ScrollerType t) override { type = t; }

		void scroll(float v) override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
		void on_gain_track_element();
		void on_lost_track_element();
		void on_gain_thumb_event_receiver();
		void on_lost_thumb_event_receiver();
		void on_gain_view();
	};

	struct cScrollViewPrivate : cScrollView // R ~ on_*
	{
		Entity* target = nullptr;
		cElementPrivate* target_element = nullptr;

		cScrollerPrivate* scroller = nullptr;

		void on_child_message(Message msg, void* p) override;
	};
}
