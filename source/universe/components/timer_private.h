#include <flame/universe/systems/timer_management.h>
#include <flame/universe/components/timer.h>

namespace flame
{
	struct cTimerPrivate : cTimer
	{
		sTimerManagement* management;
		bool timing;

		cTimerPrivate();
		~cTimerPrivate();
		void start();
		void stop();
		void on_entered_world() override;
		void on_left_world() override;
	};
}
