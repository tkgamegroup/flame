#pragma once

#include "../component.h"

namespace flame
{
	struct cAnimation : Component
	{
		inline static auto type_name = "flame::cAnimation";
		inline static auto type_hash = ch(type_name);

		cAnimation() :
			Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_model_name() const = 0;
		virtual void set_model_name(const wchar_t* src) = 0;

		virtual const wchar_t* get_src() const = 0;
		virtual void set_src(const wchar_t* src) = 0;

		virtual int get_playing() = 0;
		virtual void play(uint id) = 0;
		virtual void stop() = 0;
		virtual void stop_at(uint id, int frame) = 0;

		virtual bool get_loop() const = 0;
		virtual void set_loop(bool l) = 0;

		virtual void* add_callback(void (*callback)(Capture& c, int frame), const Capture& capture) = 0;
		virtual void remove_callback(void* cb) = 0;

		FLAME_UNIVERSE_EXPORTS static cAnimation* create(void* parms = nullptr);
	};
}
