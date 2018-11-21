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

#include <flame/type.h>

#include <stdarg.h>

namespace flame
{
	/*
		fmt:
			i    - int
			i2   - Ivec2
			i3   - Ivec3
			i4   - Ivec4
			f    - float
			f2   - Vec2
			f3   - Vec3
			f4   - Vec4
			b    - uchar
			b2   - Bvec2
			b3   - Bvec3
			b4   - Bvec4
			p    - void*
			str  - char*
			wstr - wchar_t*
			this - this pointer

		usage:
			[fmt]:[name]
	*/

	typedef void(*PF)(CommonData*);

	FLAME_FUNCTION_EXPORTS PF get_PF(unsigned int id, const char **out_filename = nullptr, int *out_line_beg = nullptr, int *out_line_end = nullptr);
	FLAME_FUNCTION_EXPORTS unsigned int get_PF_props(PF pf, const char **out_filename = nullptr, int *out_line_beg = nullptr, int *out_line_end = nullptr);
	FLAME_FUNCTION_EXPORTS void register_PF(PF pf, unsigned int id, const char *filename, int line_beg, int line_end);

	struct Function
	{
		const char *para_fmt;
		int para_cnt;
		const char *capt_fmt;
		int capt_cnt;

		PF pf;
		CommonData datas[1];

		FLAME_FUNCTION_EXPORTS void exec();
		FLAME_FUNCTION_EXPORTS void exec_in_new_thread();

		FLAME_FUNCTION_EXPORTS static Function *create(PF pf, const char *parm_fmt, const char *capt_fmt, va_list ap);
		FLAME_FUNCTION_EXPORTS static void destroy(Function *f);
	};
}

#define FLAME_REGISTER_FUNCTION_BEG(name, id) struct name{name(){register_PF(v, id, __FILE__, line_beg, line_end);}static const int line_beg = __LINE__;static void v(CommonData *d){
#define FLAME_REGISTER_FUNCTION_END(name) }static const int line_end = __LINE__;};static name name##_;
