#include <flame/universe/components/scroller.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;
	struct cLayoutPrivate;

	struct cScrollerPrivate : cScroller // R ~ on_*
	{
		float step = 1.f;

		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		cElementPrivate* htrack_element = nullptr; // R ref place=htrack
		cElementPrivate* hthumb_element = nullptr; // R ref place=hthumb
		cEventReceiverPrivate* hthumb_event_receiver = nullptr; // R ref place=hthumb

		cElementPrivate* vtrack_element = nullptr; // R ref place=vtrack
		cElementPrivate* vthumb_element = nullptr; // R ref place=vthumb
		cEventReceiverPrivate* vthumb_event_receiver = nullptr; // R ref place=vthumb

		cElementPrivate* view_element = nullptr; // R ref place=view
		cLayoutPrivate* view_layout = nullptr; // R ref place=view

		cElementPrivate* target_element = nullptr;

		void* mouse_scroll_listener = nullptr;

		void* htrack_element_listener = nullptr;
		void* hthumb_mouse_listener = nullptr;
		void* vtrack_element_listener = nullptr;
		void* vthumb_mouse_listener = nullptr;

		void scroll(const Vec2f& v) override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void on_gain_htrack_element();
		void on_lost_htrack_element();
		void on_gain_hthumb_event_receiver();
		void on_lost_hthumb_event_receiver();

		void on_gain_vtrack_element();
		void on_lost_vtrack_element();
		void on_gain_vthumb_event_receiver();
		void on_lost_vthumb_event_receiver();

		void on_gain_view_element();

		bool check_refs() override;
	};
}
