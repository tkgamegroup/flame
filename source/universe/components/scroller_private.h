#include <flame/universe/components/scroller.h>

namespace flame
{
	struct cElementPrivate;
	struct cEventReceiverPrivate;
	struct cLayoutPrivate;

	struct cScrollerPrivate : cScroller // R ~ on_*
	{
		ScrollerType type = ScrollerVertical;

		cEventReceiverPrivate* event_receiver = nullptr; // R ref
		cElementPrivate* track_element = nullptr; // R ref place=scrollbar_track
		cEventReceiverPrivate* track_event_receiver = nullptr; // R ref place=scrollbar_track
		cElementPrivate* thumb_element = nullptr; // R ref place=scrollbar_thumb
		cEventReceiverPrivate* thumb_event_receiver = nullptr; // R ref place=scrollbar_thumb

		Entity* view = nullptr;
		cLayoutPrivate* view_layout = nullptr;
		cElementPrivate* target_element = nullptr;

		void* mouse_scroll_listener = nullptr;

		ScrollerType get_type() const override { return type; }
		void set_type(ScrollerType t) override { type = t; }

		void scroll(float v) override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void on_entity_added_child(Entity* e) override;
		void on_entity_removed_child(Entity* e) override;
	};

	struct cScrollViewPrivate : cScrollView // R ~ on_*
	{
		Entity* target = nullptr;

		cScrollerPrivate* scroller = nullptr; // R ref place=parent

		void on_gain_scroller();

		void on_entity_added_child(Entity* e) override;
		void on_entity_removed_child(Entity* e) override;
	};
}
