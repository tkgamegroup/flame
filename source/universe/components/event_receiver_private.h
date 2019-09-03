#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cEventReceiverPrivate : cEventReceiver
	{
		std::vector<uint> acceptable_drops;

		//Array<Function<FoucusListenerParm>> focus_listeners$;
		std::vector<std::unique_ptr<Closure<void(void* c, KeyState action, uint value)>>> key_listeners;
		std::vector<std::unique_ptr<Closure<void(void* c, KeyState action, MouseKey key, const Vec2f & pos)>>> mouse_listeners;
		//Array<Function<DropListenerParm>> drop_listeners$;
		//Array<Function<ChangedListenerParm>> changed_listeners$;
		//Array<Function<ChildListenerParm>> child_listeners$;

		cEventReceiverPrivate();
		~cEventReceiverPrivate();
		void start();
	};
}
