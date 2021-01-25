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
			virtual void push_int(int i) = 0;
			virtual void push_float(float f) = 0;
			virtual void push_vec2(const vec2& v) = 0;
			virtual void push_vec3(const vec3& v) = 0;
			virtual void push_vec4(const vec4& v) = 0;
			virtual void push_string(const char* str) = 0;
			virtual void push_pointer(void* p) = 0;
			virtual void push_object() = 0;
			virtual void pop(uint number) = 0;
			virtual void get_global(const char* name) = 0;
			virtual void get_member(const char* name, uint idx = 0 /* when name = null */ ) = 0;
			virtual void set_object_type(const char* type_name, void* p = INVALID_POINTER) = 0;
			virtual void set_member_name(const char* name) = 0;
			virtual void set_global_name(const char* name) = 0;
			virtual void call(uint parameters_count) = 0;
			virtual bool excute(const char* str) = 0;
			virtual bool excute_file(const wchar_t* filename) = 0;

			FLAME_SCRIPT_EXPORTS static Instance* get_default();
			FLAME_SCRIPT_EXPORTS static void set_default(Instance* instance);
			FLAME_SCRIPT_EXPORTS static Instance* create();
		};
	}
}
