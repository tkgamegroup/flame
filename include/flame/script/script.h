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
		enum ScriptType
		{
			ScriptTypeInt,
			ScriptTypeFloat,
			ScriptTypePointer,
			ScriptTypeVec2f
		};

		struct Parameter
		{
			ScriptType type;
			union
			{
				void* p;
				Vec4i i;
				Vec4f f;
			}data;
		};

		struct Instance
		{
			virtual bool excute(const wchar_t* filename) = 0;
			virtual void add_object(void* p, const char* name, const char* type_name) = 0;
			virtual void call_slot(uint s, uint parameters_count, Parameter* parameters) = 0;
			virtual void release_slot(uint s) = 0;

			FLAME_SCRIPT_EXPORTS static Instance* get();
			FLAME_SCRIPT_EXPORTS static Instance* create(bool as_default = true);
		};
	}
}
