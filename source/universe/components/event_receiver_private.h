#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cEventReceiverPrivate : cEventReceiver
	{
		std::vector<uint> acceptable_drops;

		std::vector<std::unique_ptr<Closure<void(void* c, FocusType type)>>> focus_listeners;
		std::vector<std::unique_ptr<Closure<void(void* c, KeyState action, uint value)>>> key_listeners;
		std::vector<std::unique_ptr<Closure<void(void* c, KeyState action, MouseKey key, const Vec2f & value)>>> mouse_listeners;
		std::vector<std::unique_ptr<Closure<void(void* c, DragAndDrop action, cEventReceiver * er, const Vec2f & pos)>>> drag_and_drop_listeners;

		cEventReceiverPrivate();
		~cEventReceiverPrivate();
		void start();
		Component* copy();
	};
}
