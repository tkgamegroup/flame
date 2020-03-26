#include <flame/universe/systems/timer_management.h>
#include <flame/universe/components/timer.h>

namespace flame
{
	struct cTimerPrivate : cTimer
	{
		std::unique_ptr<Closure<void(void* c)>> callback;

		sTimerManagement* management;
		bool updating;

		cTimerPrivate();
		~cTimerPrivate();
		void start();
		void stop();
		void on_entered_world() override;
		void on_left_world() override;
	};
}
