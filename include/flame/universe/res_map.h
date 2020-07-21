#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct ResMap
	{
		virtual void release() = 0;

		virtual void get_res_path(const char* name, wchar_t* dst) const = 0;
		virtual void traversal(void (*callback)(Capture& c, const char* name, const wchar_t* path), const Capture& capture) const = 0;

		FLAME_UNIVERSE_EXPORTS static ResMap* create(const wchar_t* filename);
	};
}
