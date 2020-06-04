#include <flame/universe/systems/timer_management.h>
#include <flame/universe/components/timer.h>

namespace flame
{
	struct cTimerPrivate : cTimer
	{
		std::unique_ptr<Closure<void(Capture& c)>> callback;

		sTimerManagement* management;

		cTimerPrivate();
		~cTimerPrivate();
		void reset();
		void start(bool force_restart);
		void stop();
		void on_event(EntityEvent e, void* t) override;
	};
}
