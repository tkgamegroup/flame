#include <flame/universe/world.h>
#include "timer_private.h"

namespace flame
{
	cTimerPrivate::cTimerPrivate() :
		management(nullptr),
		timing(false)
	{
		callbacks.impl = ListenerHubImpl::create();
	}

	cTimerPrivate::~cTimerPrivate()
	{
		stop();
		ListenerHubImpl::destroy(callbacks.impl);
	}

	void cTimerPrivate::start()
	{
		if (!timing)
		{
			_t = 0;
			management->add_to_update_list(this);
		}
	}

	void cTimerPrivate::stop()
	{
		if (timing)
		{
			_t = 0;
			management->remove_from_update_list(this);
		}
	}

	void cTimerPrivate::on_entered_world()
	{
		management = entity->world()->get_system(sTimerManagement);
	}

	void cTimerPrivate::on_left_world()
	{
		stop();
		management = nullptr;
	}

	void cTimer::start()
	{
		((cTimerPrivate*)this)->start();
	}

	void cTimer::stop()
	{
		((cTimerPrivate*)this)->stop();
	}

	cTimer* cTimer::create()
	{
		return new cTimerPrivate();
	}
}
