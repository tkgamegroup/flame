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

#include "object.h"
#include "light.h"

#include <flame/math.h>

#include <vector>
#include <mutex>

namespace flame
{
	namespace ThreeDWorld
	{
		struct Cell
		{
			std::vector<Object*> objs;
			std::vector<PointLight*> lits;
			bool visited;
		};

		struct Cell3DArray
		{
			int size;
			int size2;
			int size3;
			float length;
			float half_length;
			float off;

			Cell *cells;

			std::mutex mtx;

			Cell3DArray(int _size, float _length) :
				size(_size),
				size2(_size * _size),
				size3(_size * _size * _size),
				length(_length),
				half_length(_length * 0.5f)
			{
				cells = new Cell[size3];
				off = size * length / 2.f;
			}

			~Cell3DArray()
			{
				delete[]cells;
			}

			void clear_objs()
			{
				for (auto i = 0; i < size3; i++)
					cells[i].objs.clear();
			}

			void clear_lits()
			{
				for (auto i = 0; i < size3; i++)
					cells[i].lits.clear();
			}

			int cell_idx(const Vec3 &pos)
			{
				return int((pos.x + off) / length) +
					int((pos.y + off) / length) * size + 
					int((pos.z + off) / length) * size2;
			}
			
			Vec3 cell_pos(int idx)
			{
				auto y = idx / size2;
				auto z = (idx % size2) / size;
				auto x = idx % size;
				return Vec3((x + 0.5f) * length,
					(y + 0.5f) * length,
					(z + 0.5f) * length) - Vec3(off);
			}

			void add_obj(Object *obj)
			{
				int x0, x1, y0, y1, z0, z1;
				x0 = (obj->aabb.min.x + off) / length;
				x1 = (obj->aabb.max.x + off) / length;
				y0 = (obj->aabb.min.y + off) / length;
				y1 = (obj->aabb.max.y + off) / length;
				z0 = (obj->aabb.min.z + off) / length;
				z1 = (obj->aabb.max.z + off) / length;

				for (auto x = x0; x <= x1; x++)
				{
					for (auto y = y0; y <= y1; y++)
					{
						for (auto z = z0; z <= z1; z++)
							cells[y * size2 + z * size + x].objs.push_back(obj);
					}
				}
			}

			void add_lit(PointLight *lit)
			{
				int x = (lit->pos.x + off) / length;
				int y = (lit->pos.y + off) / length;
				int z = (lit->pos.z + off) / length;

				cells[y * size2 + z * size + x].lits.push_back(lit);
			}

			void init_visit()
			{
				for (auto i = 0; i < size3; i++)
					cells[i].visited = false;
			}

			int front(int idx)
			{
				if ((idx % size2) > size2 - size)
					return -1;
				auto v = idx + size;
				if (v > size3)
					return -1;
				return v;
			}

			int back(int idx)
			{
				if ((idx % size2) < size)
					return -1;
				auto v = idx - size;
				if (v < 0)
					return -1;
				return v;
			}

			int left(int idx)
			{
				if ((idx % size) == 0)
					return -1;
				auto v = idx - 1;
				if (v < 0)
					return -1;
				return v;
			}

			int right(int idx)
			{
				if ((idx % size) == size - 1)
					return -1;
				auto v = idx + 1;
				if (v > size3)
					return -1;
				return v;
			}

			int up(int idx)
			{
				if (idx > size3 - size2)
					return -1;
				auto v = idx + size2;
				if (v > size3)
					return -1;
				return v;
			}

			int down(int idx)
			{
				if (idx < size2)
					return -1;
				auto v = idx - size2;
				if (v < 0)
					return -1;
				return v;
			}
		};
	}
}
