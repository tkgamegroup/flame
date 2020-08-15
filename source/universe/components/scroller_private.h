#include <flame/universe/components/scroller.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;
	struct cLayoutPrivate;

	struct cScrollerPrivate : cScroller // R ~ on_*
	{
		ScrollerType type = ScrollerVertical;

		float step = 1.f;

		cEventReceiverPrivate* event_receiver = nullptr; // R ref
		cElementPrivate* track_element = nullptr; // R ref place=scrollbar_track
		cEventReceiverPrivate* track_event_receiver = nullptr; // R ref place=scrollbar_track
		cElementPrivate* thumb_element = nullptr; // R ref place=scrollbar_thumb
		cEventReceiverPrivate* thumb_event_receiver = nullptr; // R ref place=scrollbar_thumb

		Entity* view = nullptr;
		cElementPrivate* view_element = nullptr;
		cLayoutPrivate* view_layout = nullptr;
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

		void on_child_message(Message msg, void* p) override;
	};

	struct cScrollViewPrivate : cScrollView // R ~ on_*
	{
		Entity* target = nullptr;
		cElementPrivate* target_element = nullptr;

		cScrollerPrivate* scroller = nullptr; // R ref place=parent

		void on_gain_scroller();
		void on_lost_scroller();

		void on_child_message(Message msg, void* p) override;
	};
}
