#include <flame/universe/world.h>
#include "timer_private.h"

namespace flame
{
	cTimerPrivate::cTimerPrivate() :
		management(nullptr)
	{
		interval = 0.f;
		max_time = -1.f;
		max_times = -1;

		_updating = false;
		_time = 0.f;
		_total_time = 0.f;
		_times = 0;

		callback.reset(new Closure<void(void*)>(nullptr, Mail()));
	}

	cTimerPrivate::~cTimerPrivate()
	{
		stop();
	}

	void cTimerPrivate::reset()
	{
		_time = 0.f;
		_total_time = 0.f;
		_times = 0;
	}

	void cTimerPrivate::start()
	{
		if (!_updating && callback->f)
			management->add_to_update_list(this);
	}

	void cTimerPrivate::stop()
	{
		if (_updating)
		{
			reset();
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

	void cTimer::reset()
	{
		((cTimerPrivate*)this)->reset();
	}

	void cTimer::set_callback(void(*callback)(void* c), const Mail& capture, bool _start)
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
