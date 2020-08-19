#include <flame/universe/components/tree.h>

namespace flame
{
	struct cTextPrivate;
	struct cEventReceiverPrivate;

	struct cTreePrivate : cTree // R ~ on_*
	{
		Entity* selected = nullptr;

		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;

		void expand_to_selected() override;
	};

	struct cTreeLeafPrivate : cTreeLeaf // R ~ on_*
	{
		cTreePrivate* tree = nullptr; // R ref place=ancestor optional
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};

	struct cTreeNodePrivate : cTreeNode // R ~ on_*
	{
		cTreePrivate* tree = nullptr; // R ref place=ancestor optional
		cEventReceiverPrivate* event_receiver = nullptr; // R ref
		cTextPrivate* arrow_text = nullptr; // R ref place=arrow
		cEventReceiverPrivate* arrow_event_receiver = nullptr; // R ref place=arrow
		cElementPrivate* items_element = nullptr; // R ref place=items

		void* mouse_listener = nullptr;
		void* arrow_mouse_listener = nullptr;

		void toggle_collapse() override;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
		void on_gain_arrow_event_receiver();
		void on_lost_arrow_event_receiver();
	};
}
