#include <flame/universe/components/tree.h>

namespace flame
{
	struct cEventReceiverPrivate;

	struct cTreePrivate : cTree // R ~ on_*
	{
		Entity* selected = nullptr;

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;

		void expand_to_selected() override;
	};

	struct cTreeLeafPrivate : cTreeLeaf // R ~ on_*
	{
		cEventReceiverPrivate* event_receiver = nullptr; // R ref
		cTreePrivate* tree = nullptr; // R ref place=ancestor

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};

	struct cTreeNodePrivate : cTreeNode // R ~ on_*
	{
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_listener = nullptr;

		void on_gain_event_receiver();
		void on_lost_event_receiver();
	};
}
