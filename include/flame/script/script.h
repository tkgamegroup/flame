#pragma once

#ifdef FLAME_SCRIPT_MODULE
#define FLAME_SCRIPT_EXPORTS __declspec(dllexport)
#else
#define FLAME_SCRIPT_EXPORTS __declspec(dllimport)
#endif

#include <flame/math.h>

namespace flame
{
	struct UdtInfo;

	namespace script
	{
		struct Instance
		{
			virtual bool excute(const wchar_t* filename) = 0;
			virtual void add_object(void* p, const char* name, const char* type_name) = 0;

			FLAME_SCRIPT_EXPORTS static Instance* get();
		};
	}
}
