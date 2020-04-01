#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	inline void* add_fps_listener(void (*event)(void* c, uint fps), const Mail& _capture)
	{
		struct Capture
		{
			uint last_frame;
			void (*e)(void* c, uint fps);
		}capture;
		capture.last_frame = 0;
		capture.e = event;
		auto ret = looper().add_event([](void* c, bool* go_on) {
			auto& capture = *(Capture*)c;
			auto frame = looper().frame;
			capture.e((char*)c + sizeof(Capture), frame - capture.last_frame);
			capture.last_frame = frame;
			*go_on = true;
		}, Mail::expand_original(&capture, _capture), 1.f);
		f_free(_capture.p);
		return ret;
	}
}
