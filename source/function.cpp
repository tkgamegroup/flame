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

#include <flame/math.h>
#include <flame/string.h>
#include <flame/function.h>

#include <process.h>
#include <assert.h>

namespace flame
{
	struct RegisteredPF
	{
		PF pf;
		uint id;
		const char *parm_fmt;
		const char *filename;
		int line_beg;
		int line_end;
	};

	static std::vector<RegisteredPF> pfs;

	void register_PF(PF pf, uint id, const char *parm_fmt, const char *filename, int line_beg, int line_end)
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
		r.parm_fmt = parm_fmt;
		r.filename = filename;
		r.line_beg = line_beg;
		r.line_end = line_end;
		pfs.push_back(r);
	}

	PF find_registered_PF(uint id, const char **out_parm_fmt, const char **out_filename, int *out_line_beg, int *out_line_end)
	{
		for (auto &r : pfs)
		{
			if (r.id == id)
			{
				if (out_parm_fmt)
					*out_parm_fmt = r.parm_fmt;
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

	uint find_registered_PF(PF pf, const char **out_parm_fmt, const char **out_filename, int *out_line_beg, int *out_line_end)
	{
		for (auto &r : pfs)
		{
			if (r.pf == pf)
			{
				if (out_parm_fmt)
					*out_parm_fmt = r.parm_fmt;
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

	Function *Function::create(uint id, int capt_cnt)
	{
		const char *parm_fmt;
		auto pf = find_registered_PF(id, &parm_fmt);
		auto parm_sp = string_split(std::string(parm_fmt));

		auto f = (Function*)::malloc(sizeof(Function) + sizeof(CommonData) * (parm_sp.size() + capt_cnt - 1));
		f->para_fmt = parm_fmt;
		f->para_cnt = parm_sp.size();
		f->capt_cnt = capt_cnt;
		f->pf = pf;

		return f;
	}

	Function *Function::create(PF pf, const char *parm_fmt, const std::vector<CommonData> &capt)
	{
		auto parm_sp = string_split(std::string(parm_fmt));

		auto f = (Function*)::malloc(sizeof(Function) + sizeof(CommonData) * (parm_sp.size() + capt.size() - 1));
		f->para_fmt = parm_fmt;
		f->para_cnt = parm_sp.size();
		f->capt_cnt = capt.size();
		f->pf = pf;

		auto d = f->datas;
		for (auto &t : parm_sp)
		{
			d->set_fmt(t.c_str());

			d++;
		}
		for (auto i = 0; i < capt.size(); i++)
		{
			*d = capt[i];

			d++;
		}

		return f;
	}

	void Function::destroy(Function *f)
	{
		::free(f);
	}
}
