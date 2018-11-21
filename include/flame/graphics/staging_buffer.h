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

#pragma once

#include <flame/graphics/buffer.h>
#include <flame/graphics/commandbuffer.h>

#include <vector>
#include <map>
#include <algorithm>

namespace flame
{
	namespace graphics
	{
		struct Buffer;

		struct StagingBuffer
		{
			Buffer *v;
			int curr_size;

			std::map<Buffer*, std::vector<BufferCopy>> copies;

			inline StagingBuffer(Device *d, int size)
			{
				v = graphics::create_buffer(d, size, graphics::BufferUsageTransferSrc,
					graphics::MemPropHost | graphics::MemPropHostCoherent);
				v->map();
				curr_size = 0;
			}

			inline ~StagingBuffer()
			{
				graphics::destroy_buffer(v);
			}

			inline void update(Buffer *b, int off, int size, void *data = nullptr)
			{
				auto it = copies.find(b);
				if (it == copies.end())
					it = copies.insert(std::make_pair(b, std::vector<BufferCopy>())).first;

				if (data)
					memcpy((void*)((unsigned long long)v->mapped + curr_size), data, size);

				BufferCopy cpy;
				cpy.size = size;
				cpy.src_offset = curr_size;
				cpy.dst_offset = off;
				it->second.push_back(cpy);

				curr_size += size;
			}

			inline void flush(Commandbuffer *cb)
			{
				for (auto &it : copies)
					cb->copy_buffer(v, it.first, it.second.size(), it.second.data());
				copies.clear();
			}
		};
	}
}
