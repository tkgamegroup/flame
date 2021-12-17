#include "foundation_private.h"
#include "system_private.h"
#include "window_private.h"
#include "application.h"

namespace flame
{
	std::map<std::wstring, std::filesystem::path> Path::map;

	struct _Initializer
	{
		_Initializer()
		{
			auto p = getenv("FLAME_PATH");
			assert(p);
			std::filesystem::path engine_path = p;
			engine_path.make_preferred();
			engine_path /= L"default_assets";
			Path::add_root(engine_path.filename(), engine_path);
		}
	};
	static _Initializer _initializer;

	uint frames = 0;
	uint fps = 0;
	float delta_time = 0.f;
	float total_time = 0.f;

	static uint64 last_time = 0;
	static float fps_delta = 0.f;
	static uint fps_counting = 0;

	struct Event
	{
		float interval;
		float rest;
		std::function<bool()> callback;
	};

	static std::list<Event> events;
	static std::recursive_mutex event_mtx;

	int run(const std::function<bool()>& callback)
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
				auto w = *it;
				if (w->dead)
				{
					it = windows.erase(it);
					delete w;
				}
				else
					it++;
			}

			if (windows.empty())
				return 0;

			{
				std::lock_guard<std::recursive_mutex> lock(event_mtx);
				for (auto it = events.begin(); it != events.end();)
				{
					it->rest -= delta_time;
					if (it->rest <= 0)
					{
						if (!it->callback())
						{
							it = events.erase(it);
							continue;
						}
						it->rest = it->interval;
					}
					it++;
				}
			}

			if (!callback())
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

	void* add_event(const std::function<bool()>& callback, float time)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		auto& e = events.emplace_back();
		e.interval = time;
		e.rest = time;
		e.callback = callback;
		return &e;
	}

	void reset_event(void* _ev)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		auto ev = (Event*)_ev;
		ev->rest = ev->interval;
	}

	void remove_event(void* ev)
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		for (auto it = events.begin(); it != events.end(); it++)
		{
			if (&*it == ev)
			{
				events.erase(it);
				break;
			}
		}
	}

	void clear_events()
	{
		std::lock_guard<std::recursive_mutex> lock(event_mtx);
		events.clear();
	}
}
