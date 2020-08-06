#include <flame/universe/components/menu.h>

namespace flame
{
	struct EntityPrivate;
	struct cEventReceiverPrivate;

	struct cMenuPrivate : cMenu // R ~ on_*
	{
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_down_listener = nullptr;
		void* root_listener = nullptr;
		
		std::unique_ptr<EntityPrivate, Delector> items;
		EntityPrivate* root = nullptr;
		bool opened = false;

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void set_items(Entity* e) override { items.reset((EntityPrivate*)e); }
	};
}
