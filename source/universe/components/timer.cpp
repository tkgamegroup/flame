#include <flame/universe/world.h>
#include "timer_private.h"

namespace flame
{
	cTimerPrivate::cTimerPrivate() :
		management(nullptr)
	{
		interval = 0.f;

		_time = -1.f;

		callback.reset(new Closure<void(Capture&)>(nullptr, Capture()));
	}

	cTimerPrivate::~cTimerPrivate()
	{
		stop();
	}

	void cTimerPrivate::reset()
	{
		_time = -1.f;
	}

	void cTimerPrivate::start(bool force_restart)
	{
		if (_time != -1.f)
		{
			if (force_restart)
				reset();
		}
		else if (callback->f)
			management->add_to_update_list(this);
	}

	void cTimerPrivate::stop()
	{
		if (_time != -1.f)
		{
			management->remove_from_update_list(this);
			reset();
		}
	}

	void cTimerPrivate::on_event(Entity::Event e, void* t)
	{
		switch (e)
		{
		case Entity::EventEnteredWorld:
			management = entity->world->get_system(sTimerManagement);
		break;
		case Entity::EventLeftWorld:
			stop();
			management = nullptr;
			break;
		case Entity::EventVisibilityChanged:
			if (!entity->global_visibility)
				stop();
			break;
		}
	}

	void cTimer::start(bool force_restart)
	{
		((cTimerPrivate*)this)->start(force_restart);
	}

	void cTimer::stop()
	{
		((cTimerPrivate*)this)->stop();
	}

	void cTimer::reset()
	{
		((cTimerPrivate*)this)->reset();
	}

	void cTimer::set_callback(void(*callback)(Capture& c), const Capture& capture, bool _start)
	{
		stop();
		((cTimerPrivate*)this)->callback.reset(new Closure(callback, capture));
		if (_start)
			start();
	}

	cTimer* cTimer::create()
	{
		return new cTimerPrivate();
	}
}
