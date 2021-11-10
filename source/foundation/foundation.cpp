#include "foundation_private.h"
#include "system_private.h"
#include "window_private.h"

namespace flame
{
	void* f_malloc(uint size)
	{
		return malloc(size);
	}

	void* f_realloc(void* p, uint size)
	{
		return realloc(p, size);
	}

	void f_free(void* p)
	{
		free(p);
	}

#ifdef _DEBUG
	std::vector<std::unique_ptr<Closure<void(Capture&)>>> assert_callbacks;
#endif

	void* add_assert_callback(void (*callback)(Capture& c), const Capture& capture)
	{
#ifdef _DEBUG
		auto c = new Closure(callback, capture);
		assert_callbacks.emplace_back(c);
		return c;
#endif
	}

	void remove_assert_callback(void* ret)
	{
#ifdef _DEBUG
		std::erase_if(assert_callbacks, [&](const auto& i) {
			return i == (decltype(i))ret;
		});
#endif
	}

	void raise_assert(const char* expression, const char* file, uint line)
	{
#ifdef _DEBUG
		for (auto& c : assert_callbacks)
			c->call();
		_wassert(s2w(expression).c_str(), s2w(file).c_str(), line);
#endif
	}

	uint frames = 0;
	static uint64 last_time = 0;
	float delta_time = 0.f;
	float total_time = 0.f;
	uint fps = 0;
	static float fps_delta = 0.f;
	static uint fps_counting = 0;

	std::vector<std::unique_ptr<NativeWindowPrivate>> windows;

	uint get_frames()
	{
		return frames;
	}

	float get_delta_time()
	{
		return delta_time;
	}

	float get_total_time()
	{
		return total_time;
	}

	uint get_fps()
	{
		return fps;
	}

	NativeWindow* get_window(uint idx)
	{
		return windows[idx].get();
	}

	struct Event
	{
		uint id;
		float interval;
		float rest;
		void(*callback)(Capture& c);
		Capture capture;

		~Event()
		{
			f_free(capture._data);
		}
	};

	static std::list<std::unique_ptr<Event>> events;
	static std::recursive_mutex event_mtx;

	int run(void (*callback)(Capture& c, float delta_time), const Capture& capture)
	{
		if (!callback)
		{
			for (;;)
			{
				MSG msg;
				while (GetMessageW(&msg, NULL, 0, 0))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
		}

		if (windows.empty())
			return 1;

		last_time = get_now_ns();
		frames = 0;

		for (;;)
		{
			MSG msg;
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			for (auto it = windows.begin(); it != windows.end(); )
			{
				auto w = it->get();

				if (w->dead)
					it = windows.erase(it);
				else
					it++;
			}

			if (windows.empty())
			{
				f_free(capture._data);
				return 0;
			}

			{
				std::lock_guard<std::recursive_mutex> lock(event_mtx);
				for (auto it = events.begin(); it != events.end();)
				{
					auto& e = *it;
					auto excute = false;
					e->rest -= delta_time;
					if (e->rest <= 0)
						excute = true;
					if (excute)
					{
						e->capture._current = INVALID_POINTER;
						e->callback(e->capture);
						if (e->capture._current == INVALID_POINTER)
						{
							it = events.erase(it);
							continue;
						}
						e->rest = e->interval;
					}
					it++;
				}
			}

			callback((Capture&)capture, delta_time);
			if (capture._current == INVALID_POINTER)
				return 0;

			frames++;
			auto et = last_time;
			last_time = get_now_ns();
			et = last_time - et;
			delta_time = et / 1000000000.f;
			total_time += delta_time;
			fps_counting++;
			fps_delta += delta_time;
			if (fps_delta >= 1.f)
			{
				fps = fps_counting;
				fps_counting = 0;
				fps_delta = 0.f;
			}
		}
	}

	void* add_event(void (*callback)(Capture& c), const Capture& capture, float time, uint id)
	{
		event_mtx.lock();
		auto e = new Event;
		e->id = id;
		e->interval = time;
		e->rest = time;
		e->callback = callback;
		e->capture = capture;
		events.emplace_back(e);
		event_mtx.unlock();
		return e;
	}

	void reset_event(void* _ev)
	{
		auto ev = (Event*)_ev;
		ev->rest = ev->interval;
	}

	void remove_event(void* ev)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		for (auto it = events.begin(); it != events.end(); it++)
		{
			if ((*it).get() == ev)
			{
				if ((*it)->capture._current != nullptr)
					(*it)->capture._current = INVALID_POINTER;
				else
					events.erase(it);
				break;
			}
		}
	}

	void clear_events(int id)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		if (id == -1)
		{
			events.clear();
			return;
		}
		for (auto it = events.begin(); it != events.end();)
		{
			if ((*it)->id == id)
				it = events.erase(it);
			else
				it++;
		}
	}
}
