#include <flame/universe/world.h>
#include "timer_private.h"

namespace flame
{
	cTimerPrivate::cTimerPrivate() :
		management(nullptr),
		updating(false)
	{
		interval = 0.f;
		max_time = -1.f;
		max_times = -1;

		_time = 0.f;
		_total_time = 0.f;
		_times = 0;

		auto c = new Closure<void(void* c)>;
		c->function = nullptr;
		callback.reset(c);
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
		if (!updating && callback->function)
			management->add_to_update_list(this);
	}

	void cTimerPrivate::stop()
	{
		if (updating)
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

	void cTimer::set_callback(void(*callback)(void* c), const Mail<>& capture, bool _start)
	{
		stop();
		auto c = new Closure<void(void* c)>;
		c->function = callback;
		c->capture = capture;
		((cTimerPrivate*)this)->callback.reset(c);
		if (_start)
			start();
	}

	cTimer* cTimer::create()
	{
		return new cTimerPrivate();
	}
}
