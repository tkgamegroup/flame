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

#include <flame/type.h>
#include <flame/memory.h>

namespace flame
{
	template<typename T>
	struct Array
	{
		int size;
		T *v;

		inline void resize(int new_size)
		{
			if (size == new_size)
				return;

			if (size > new_size)
			{
				for (auto i = new_size; i < size; i++)
					v[i].~T();
			}

			v = new_size == 0 ? nullptr : (T*)realloc(v, sizeof(T) * new_size);

			if (new_size > size)
			{
				for (auto i = size; i < new_size; i++)
					new(&v[i])T();
			}

			size = new_size;
		}

		inline Array()
		{
			size = 0;
			v = nullptr;
		}

		inline ~Array()
		{
			free(v);
		}

		inline T &operator[](int idx)
		{
			return v[idx];
		}

		inline const T &operator[](int idx) const
		{
			return v[idx];
		}

		inline void insert(int pos, const T &_v)
		{
			resize(size + 1);
			v[pos] = _v;
		}

		inline void push_back(const T &_v)
		{
			insert(size, _v);
		}

		inline void remove(int idx, int count = 1)
		{
			auto new_size = size - count;
			for (auto i = idx; i < new_size; i++)
				v[i] = v[i + count];
			resize(new_size);
		}

		inline int find(const T &_v)
		{
			for (auto i = 0; i < size; i++)
			{
				if (v[i] == _v)
					return i;
			}
			return -1;
		}
	};
}
