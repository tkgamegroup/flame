//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#ifdef FLAME_WINDOWS
#ifdef FLAME_FUNCTION_MODULE
#define FLAME_FUNCTION_EXPORTS __declspec(dllexport)
#else
#define FLAME_FUNCTION_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_FUNCTION_EXPORTS
#endif

#include <flame/math.h>

#include <vector>

namespace flame
{
	struct ParmPackage
	{
		CommonData *d;
	};

	typedef void(*PF)(const ParmPackage &p);

	struct RegisteredFunction
	{
		FLAME_FUNCTION_EXPORTS uint id() const;
		FLAME_FUNCTION_EXPORTS PF pf() const;
		FLAME_FUNCTION_EXPORTS int parm_count() const;
		FLAME_FUNCTION_EXPORTS const char *filename() const;
		FLAME_FUNCTION_EXPORTS int line_beg() const;
		FLAME_FUNCTION_EXPORTS int line_end() const;
	};

	FLAME_FUNCTION_EXPORTS void register_function(uint id, PF pf, int parm_count, const char *filename, int line_beg, int line_end);
	FLAME_FUNCTION_EXPORTS RegisteredFunction *find_registered_function(uint id);
	FLAME_FUNCTION_EXPORTS RegisteredFunction *find_registered_function(PF pf);

	struct Function
	{
		int capt_cnt;

		PF pf;
		ParmPackage p;

		inline void exec() 
		{
			pf(p);
		}
		FLAME_FUNCTION_EXPORTS void thread_exec(); // function will be destroyed after thread

		FLAME_FUNCTION_EXPORTS static Function *create(uint id, const std::vector<CommonData> &capt);
		FLAME_FUNCTION_EXPORTS static Function *create(PF pf, int parm_count, const std::vector<CommonData> &capt);
		FLAME_FUNCTION_EXPORTS static void destroy(Function *f);
	};
}

#define FLAME_PARM_PACKAGE_BEGIN(name) \
	struct name : ParmPackage\
	{\
		enum { BASE = __COUNTER__ + 1 };
#define FLAME_PARM_PACKAGE_PARM(t, n, tf) \
		inline t &n()\
		{\
			return (t&)d[__COUNTER__ - BASE].tf();\
		}
#define FLAME_PARM_PACKAGE_PARM_SIZE \
		enum { PARM_SIZE = __COUNTER__ - BASE };
#define FLAME_PARM_PACKAGE_DEFAULT_CAPT(t, n, tf) \
		inline t &n()\
		{\
			return (t&)d[__COUNTER__ - BASE - 1].tf();\
		}
#define FLAME_PARM_PACKAGE_END \
		enum { SIZE = __COUNTER__ - BASE - 1 };\
	};

#define FLAME_DATA_PACKAGE_BEGIN(name, package) \
	struct name : package\
	{\
		enum { BASE = __COUNTER__ + 1 };\
		enum { P_SIZE = package::SIZE };
#define FLAME_DATA_PACKAGE_CAPT(t, n, tf) \
		inline t &n()\
		{\
			return (t&)d[__COUNTER__ - BASE + P_SIZE].tf();\
		}
#define FLAME_DATA_PACKAGE_END \
	};

#define FLAME_REGISTER_FUNCTION_BEG(name, id, package) \
	struct name\
	{\
		name()\
		{\
			register_function(id, v, package::PARM_SIZE, __FILE__, line_beg, line_end);\
		}\
		static const int line_beg = __LINE__;\
		static void v(const ParmPackage &_p)\
		{\
			auto &p = (package&)_p;
#define FLAME_REGISTER_FUNCTION_END(name) \
		}\
		static const int line_end = __LINE__;\
	};\
	static name name##_;
