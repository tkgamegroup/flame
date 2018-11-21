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

#include "math.h"
#include "string.h"
#include "function.h"

#include <process.h>
#include <assert.h>

namespace flame
{
	struct RegisteredPF
	{
		PF pf;
		unsigned int id;
		char *filename;
		int line_beg;
		int line_end;
	};

	static std::vector<RegisteredPF> pfs;

	PF get_PF(unsigned int id, const char **out_filename, int *out_line_beg, int *out_line_end)
	{
		for (auto &r : pfs)
		{
			if (r.id == id)
			{
				if (out_filename)
					*out_filename = r.filename;
				if (out_line_beg)
					*out_line_beg = r.line_beg;
				if (out_line_end)
					*out_line_end = r.line_end;
				return r.pf;
			}
		}
		return nullptr;
	}

	unsigned int get_PF_props(PF pf, const char **out_filename, int *out_line_beg, int *out_line_end)
	{
		for (auto &r : pfs)
		{
			if (r.pf == pf)
			{
				if (out_filename)
					*out_filename = r.filename;
				if (out_line_beg)
					*out_line_beg = r.line_beg;
				if (out_line_end)
					*out_line_end = r.line_end;
				return r.id;
			}
		}
		return 0;
	}

	void register_PF(PF pf, unsigned int id, const char *filename, int line_beg, int line_end)
	{
		assert(id);
		for (auto &r : pfs)
		{
			if (r.id == id)
				assert(0);
		}
		RegisteredPF r;
		r.pf = pf;
		r.id = id;
		r.filename = (char*)filename;
		r.line_beg = line_beg;
		r.line_end = line_end;
		pfs.push_back(r);
	}

	void Function::exec()
	{
		pf(datas);
	}

	static void thread(void *p)
	{
		auto thiz = (Function*)p;
		thiz->pf(thiz->datas);
	}

	void Function::exec_in_new_thread()
	{
		_beginthread(thread, 0, this);
	}

	Function *Function::create(PF pf, const char *parm_fmt, const char *capt_fmt, va_list ap)
	{
		auto parm_sp = string_split(std::string(parm_fmt));

		auto capt_sp = string_split(std::string(capt_fmt));

		auto f = (Function*)::malloc(sizeof(Function) + sizeof(CommonData) * (parm_sp.size() + capt_sp.size() - 1));
		f->para_fmt = parm_fmt;
		f->para_cnt = parm_sp.size();
		f->capt_fmt = capt_fmt;
		f->capt_cnt;
		f->pf = pf;

		auto d = f->datas + parm_sp.size();
		for (auto &i : capt_sp)
		{
			auto sp = string_split(i, ':');
			assert(sp.size() == 2);

			auto t = sp[0];

			if (t == "i")
				d->i[0] = va_arg(ap, int);
			else if (t == "i2")
			{
				auto v = va_arg(ap, Ivec2);
				d->i[0] = v.x;
				d->i[1] = v.y;
			}
			else if (t == "i3")
			{
				auto v = va_arg(ap, Ivec3);
				d->i[0] = v.x;
				d->i[1] = v.y;
				d->i[2] = v.z;
			}
			else if (t == "i4")
			{
				auto v = va_arg(ap, Ivec4);
				d->i[0] = v.x;
				d->i[1] = v.y;
				d->i[2] = v.z;
				d->i[3] = v.w;
			}
			else if (t == "f")
				d->f[0] = va_arg(ap, double);
			else if (t == "f2")
			{
				auto v = va_arg(ap, Vec2);
				d->f[0] = v.x;
				d->f[1] = v.y;
			}
			else if (t == "f3")
			{
				auto v = va_arg(ap, Vec3);
				d->f[0] = v.x;
				d->f[1] = v.y;
				d->f[2] = v.z;
			}
			else if (t == "f4")
			{
				auto v = va_arg(ap, Vec4);
				d->f[0] = v.x;
				d->f[1] = v.y;
				d->f[2] = v.z;
				d->f[3] = v.w;
			}
			else if (t == "b")
				d->b[0] = va_arg(ap, uchar);
			else if (t == "b2")
			{
				auto v = va_arg(ap, Bvec2);
				d->b[0] = v.x;
				d->b[1] = v.y;
			}
			else if (t == "b3")
			{
				auto v = va_arg(ap, Bvec3);
				d->b[0] = v.x;
				d->b[1] = v.y;
				d->b[2] = v.z;
			}
			else if (t == "b4")
			{
				auto v = va_arg(ap, Bvec4);
				d->b[0] = v.x;
				d->b[1] = v.y;
				d->b[2] = v.z;
				d->b[3] = v.w;
			}
			else if (t == "p" || t == "str" || t == "wstr" || t == "this")
				d->p = va_arg(ap, void*);

			d++;
		}

		return f;
	}

	void Function::destroy(Function *f)
	{
		::free(f);
	}
}
