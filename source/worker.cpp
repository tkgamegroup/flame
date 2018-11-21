// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/worker.h>
#include <flame/function.h>

#include <mutex>
#include <vector>

namespace flame
{
	const auto all_workers = 3;
	static std::mutex mtx;
	static std::condition_variable cv;
	static auto workers = all_workers;

	static std::vector<Function*> works;

	static void do_work(CommonData *d)
	{
		((Function*)d[0].p)->exec();

		Function::destroy((Function*)d[0].p);
		Function::destroy((Function*)d[1].p);

		mtx.lock();
		workers++;
		cv.notify_one();
		mtx.unlock();

		try_distribute();
	}

	void try_distribute()
	{
		mtx.lock();
		if (!works.empty() && workers > 0)
		{
			workers--;
			auto w = works.front();
			works.erase(works.begin());

			auto f_thread = Function::create(do_work, "p:work p:thread", "", 0);
			f_thread->datas[0].p = w;
			f_thread->datas[1].p = f_thread;
			f_thread->exec_in_new_thread();
		}
		mtx.unlock();
	}

	void add_work(PF pf, char *capture_fmt, ...)
	{
		va_list ap;
		va_start(ap, capture_fmt);
		auto f = Function::create(pf, "", capture_fmt, ap);
		va_end(ap);

		mtx.lock();
		works.push_back(f);
		mtx.unlock();

		try_distribute();
	}

	void clear_works()
	{
		std::unique_lock<std::mutex> lk(mtx);

		works.clear();
		while (workers != all_workers)
			cv.wait(lk);
	}
}
