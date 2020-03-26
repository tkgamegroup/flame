#include <flame/universe/world.h>
#include "timer_private.h"

namespace flame
{
	cTimerPrivate::cTimerPrivate() :
		management(nullptr),
		updating(false)
	{
		_t = 0.f;
		interval = 0.f;
		times = -1;
		duration = -1.f;

		auto c = new Closure<void(void* c)>;
		c->function = nullptr;
		callback.reset(c);
	}

	cTimerPrivate::~cTimerPrivate()
	{
		stop();
	}

	void cTimerPrivate::start()
	{
		if (!updating && callback->function)
		{
			_t = 0; 
			management->add_to_update_list(this);
		}
	}

	void cTimerPrivate::stop()
	{
		if (updating)
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

	void cTimer::set_callback(void(*callback)(void* c), const Mail<>& capture)
	{
		stop();
		auto c = new Closure<void(void* c)>;
		c->function = callback;
		c->capture = capture;
		((cTimerPrivate*)this)->callback.reset(c);
	}

	cTimer* cTimer::create()
	{
		return new cTimerPrivate();
	}
}
