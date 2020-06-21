#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	inline void* add_fps_listener(void (*callback)(Capture& c, uint fps), const Capture& _capture)
	{
		struct Capturing
		{
			uint last_frame;
			void (*callback)(Capture& c, uint fps);
		}capture;
		capture.last_frame = 0;
		capture.callback = callback;
		auto ret = get_looper()->add_event([](Capture& c) {
			auto& capture = c.data<Capturing>();
			auto frame = get_looper()->frame;
			capture.callback(c.release<Capturing>(), frame - capture.last_frame);
			capture.last_frame = frame;
			c._current = INVALID_POINTER;
		}, Capture().absorb(&capture, _capture, true), 1.f);
		return ret;
	}
}
