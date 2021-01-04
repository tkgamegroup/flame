#pragma once

#ifdef FLAME_SCRIPT_MODULE
#define FLAME_SCRIPT_EXPORTS __declspec(dllexport)
#else
#define FLAME_SCRIPT_EXPORTS __declspec(dllimport)
#endif

#include <flame/math.h>

namespace flame
{
	namespace script
	{
		struct Instance
		{
			virtual bool excute(const wchar_t* filename) = 0;
			virtual void push_string(const char* str, const char* member_name = nullptr) = 0;
			virtual void push_object(const char* member_name = nullptr) = 0;
			virtual void set_global_name(const char* name) = 0;
			virtual void set_object_type(const char* type_name) = 0;

			FLAME_SCRIPT_EXPORTS static Instance* get_default();
			FLAME_SCRIPT_EXPORTS static void set_default(Instance* instance);
			FLAME_SCRIPT_EXPORTS static Instance* create();
		};
	}
}
