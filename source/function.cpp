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
	struct RegisteredFunctionPrivate : RegisteredFunction
	{
		uint id;
		PF pf;
		int parm_count;
		const char *filename;
		int line_beg;
		int line_end;
	};

	uint RegisteredFunction::id() const
	{
		return ((RegisteredFunctionPrivate*)this)->id;
	}

	PF RegisteredFunction::pf() const
	{
		return ((RegisteredFunctionPrivate*)this)->pf;
	}

	int RegisteredFunction::parm_count() const
	{
		return ((RegisteredFunctionPrivate*)this)->parm_count;
	}

	const char *RegisteredFunction::filename() const
	{
		return ((RegisteredFunctionPrivate*)this)->filename;
	}

	int RegisteredFunction::line_beg() const
	{
		return ((RegisteredFunctionPrivate*)this)->line_beg;
	}

	int RegisteredFunction::line_end() const
	{
		return ((RegisteredFunctionPrivate*)this)->line_end;
	}

	static std::vector<RegisteredFunctionPrivate*> pfs;

	void register_function(uint id, PF pf, int parm_count, const char *filename, int line_beg, int line_end)
	{
		assert(id);
		for (auto &r : pfs)
		{
			if (r->id == id)
				assert(0);
		}

		auto r = new RegisteredFunctionPrivate;
		r->id = id;
		r->pf = pf;
		r->parm_count = parm_count;
		r->filename = filename;
		r->line_beg = line_beg;
		r->line_end = line_end;
		pfs.push_back(r);
	}

	RegisteredFunction *find_registered_function(uint id)
	{
		for (auto &r : pfs)
		{
			if (r->id == id)
				return r;
		}
		return nullptr;
	}

	RegisteredFunction *find_registered_function(PF pf)
	{
		for (auto &r : pfs)
		{
			if (r->pf == pf)
				return r;
		}
		return 0;
	}

	static void thread(void *p)
	{
		auto f = (Function*)p;
		f->exec();
		Function::destroy(f);
	}

	void Function::thread_exec()
	{
		_beginthread(thread, 0, this);
	}

	Function *Function::create(uint id, const std::vector<CommonData> &capt)
	{
		auto r = (RegisteredFunctionPrivate*)find_registered_function(id);

		auto f = new Function;
		f->capt_cnt = capt.size();
		f->pf = r->pf;
		f->p.d = new CommonData[r->parm_count + capt.size()];

		auto d = f->p.d + r->parm_count;
		for (auto i = 0; i < capt.size(); i++)
		{
			*d = capt[i];

			d++;
		}

		return f;
	}

	Function *Function::create(PF pf, int parm_count, const std::vector<CommonData> &capt)
	{
		auto f = new Function;
		f->capt_cnt = capt.size();
		f->pf = pf;
		f->p.d = new CommonData[parm_count + capt.size()];

		auto d = f->p.d + parm_count;
		for (auto i = 0; i < capt.size(); i++)
		{
			*d = capt[i];

			d++;
		}

		return f;
	}

	void Function::destroy(Function *f)
	{
		delete f->p.d;
		delete f;
	}
}
