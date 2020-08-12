#include <flame/universe/components/menu.h>

namespace flame
{
	struct EntityPrivate;
	struct cElementPrivate;
	struct cEventReceiverPrivate;

	struct cMenuPrivate : cMenu // R ~ on_*
	{
		cElementPrivate* element = nullptr; // R ref
		cEventReceiverPrivate* event_receiver = nullptr; // R ref

		void* mouse_down_listener = nullptr;
		void* mouse_move_listener = nullptr;
		void* root_mouse_listener = nullptr;
		
		MenuType type = MenuTop;
		std::unique_ptr<EntityPrivate, Delector> items;
		cElementPrivate* items_element = nullptr;
		EntityPrivate* root = nullptr;
		cEventReceiverPrivate* root_event_receiver = nullptr;
		bool opened = false;
		int frame = -1;

		MenuType get_type() const override { return type; }
		void set_type(MenuType t) override { type = t; }

		void set_items(Entity* e) override;

		void open();
		void close();

		void on_gain_event_receiver();
		void on_lost_event_receiver();

		void on_entity_entered_world() override;
		void on_entity_left_world() override;
	};
}
