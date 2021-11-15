#pragma once

#include "buffer.h"
#include "command.h"

namespace flame
{
	namespace graphics
	{
		inline AccessFlags u2a(BufferUsageFlags u)
		{
			switch (u)
			{
			case BufferUsageVertex:
				return AccessVertexAttributeRead;
			case BufferUsageIndex:
				return AccessIndexRead;
			case BufferUsageIndirect:
				return AccessIndirectCommandRead;
			}
			return AccessNone;
		}

		inline PipelineStageFlags u2s(BufferUsageFlags u)
		{
			switch (u)
			{
			case BufferUsageVertex:
			case BufferUsageIndex:
				return PipelineStageVertexInput;
			case BufferUsageIndirect:
				return PipelineStageDrawIndirect;
			}
			return PipelineStageAllCommand;
		}

		template <class T>
		struct SequentialBuffer
		{
			uint capacity;
			AccessFlags access;
			PipelineStageFlags plstg;
			T* pstag = nullptr;
			uint stag_num = 0;

			UniPtr<Buffer> buf;
			UniPtr<Buffer> stagbuf;

			void rebuild()
			{
				T* temp = nullptr;
				auto n = 0;
				if (stag_num > 0)
				{
					n = stag_num;
					temp = new T[n];
					memcpy(temp, stagbuf->get_mapped(), sizeof(T) * n);
				}
				auto size = capacity * sizeof(T);
				buf->recreate(size);
				stagbuf->recreate(size);
				stagbuf->map();
				pstag = (T*)stagbuf->get_mapped();
				if (temp)
				{
					push(n, temp);
					delete[]temp;
				}
			}

			void create(BufferUsageFlags usage, uint _capacity)
			{
				capacity = _capacity;
				access = u2a(usage);
				plstg = u2s(usage);
				auto size = capacity * sizeof(T);
				buf.reset(Buffer::create(nullptr, size, BufferUsageTransferDst | usage, MemoryPropertyDevice));
				stagbuf.reset(Buffer::create(nullptr, size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stagbuf->map();
				pstag = (T*)stagbuf->get_mapped();
			}

			void push(uint cnt, const T* p)
			{
				assert(stag_num + cnt <= capacity);
				//if (stag_num + cnt > capacity)
				//{
				//	capacity = (stag_num + cnt) * 2;
				//	rebuild();
				//}

				memcpy(pstag + stag_num, p, sizeof(T) * cnt);
				stag_num += cnt;
			}

			T* stag(uint cnt)
			{
				assert(stag_num + cnt <= capacity);
				//if (stag_num + cnt > capacity)
				//{
				//	capacity = (stag_num + cnt) * 2;
				//	rebuild();
				//}

				auto dst = pstag + stag_num;
				stag_num += cnt;
				return dst;
			}

			void upload(CommandBuffer* cb)
			{
				if (stag_num == 0)
					return;
				BufferCopy cpy;
				cpy.size = stag_num * sizeof(T);
				cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
				cb->buffer_barrier(buf.get(), AccessTransferWrite, access, PipelineStageTransfer, plstg);
				stag_num = 0;
			}
		};

		template <class T>
		struct SparseBuffer
		{
			uint capacity;
			AccessFlags access;
			PipelineStageFlags plstg;
			uint n0 = 0;
			uint n1 = 0;

			UniPtr<Buffer> buf;
			UniPtr<Buffer> stagbuf;
			uint stag_capacity;
			T* pstag = nullptr;

			void create(BufferUsageFlags usage, uint _capacity)
			{
				capacity = _capacity;
				access = u2a(usage);
				plstg = u2s(usage);
				auto size = capacity * sizeof(T);
				buf.reset(Buffer::create(nullptr, size, BufferUsageTransferDst | usage, MemoryPropertyDevice));
				stag_capacity = 100;
				stagbuf.reset(Buffer::create(nullptr, sizeof(T) * stag_capacity, BufferUsageTransferSrc, MemoryPropertyHost |
					MemoryPropertyCoherent));
				pstag = (T*)stagbuf->map();
			}

			T* alloc(uint n)
			{
				assert(n0 == n1);
				assert(n1 + n <= capacity);
				if (stag_capacity < n)
				{
					stag_capacity = n;
					stagbuf->recreate(n * sizeof(T));
					pstag = (T*)stagbuf->map();
				}
				n1 += n;
				return pstag;
			}

			void free(T* p)
			{
				// TODO
			}

			void upload(CommandBuffer* cb)
			{
				if (n1 > n0)
				{
					BufferCopy cpy;
					cpy.size = (n1 - n0) * sizeof(T);
					cpy.dst_off = n0 * sizeof(T);
					cb->copy_buffer(stagbuf.get(), buf.get(), 1, &cpy);
					cb->buffer_barrier(buf.get(), AccessTransferWrite, access, PipelineStageTransfer, plstg);
					n0 = n1;
				}
			}
		};

		template <class T>
		struct StorageBuffer
		{
			T* pstag = nullptr;

			UniPtr<Buffer> buf;
			UniPtr<Buffer> stagbuf;

			std::vector<BufferCopy> cpies;

			void create(BufferUsageFlags usage)
			{
				buf.reset(Buffer::create(nullptr, sizeof(T), BufferUsageTransferDst | usage, MemoryPropertyDevice));
				stagbuf.reset(Buffer::create(nullptr, sizeof(T), BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				pstag = (T*)stagbuf->map();
			}

			void cpy_whole()
			{
				assert(cpies.empty());
				BufferCopy cpy;
				cpy.size = sizeof(T);
				cpies.push_back(cpy);
			}

			void upload(CommandBuffer* cb)
			{
				if (cpies.empty())
					return;
				cb->copy_buffer(stagbuf.get(), buf.get(), cpies.size(), cpies.data());
				cb->buffer_barrier(buf.get(), AccessTransferWrite, AccessShaderRead, PipelineStageTransfer);
				cpies.clear();
			}
		};

		template <class T>
		struct ArrayStorageBuffer : StorageBuffer<T>
		{
			using StorageBuffer<T>::pstag;
			using StorageBuffer<T>::cpies;

			auto& item(uint idx, bool mark_cpy = true)
			{
				auto& [items] = *pstag;
				assert(idx < countof(items));

				auto& item = items[idx];

				if (mark_cpy)
				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = idx * sizeof(item);
					cpy.size = sizeof(item);
					cpies.push_back(cpy);
				}

				return item;
			}
		};

		template <class T>
		struct SequentialArrayStorageBuffer : ArrayStorageBuffer<T>
		{
			using StorageBuffer<T>::pstag;
			using StorageBuffer<T>::cpies;
			using ArrayStorageBuffer<T>::item;

			uint n = 0;

			auto& add_item()
			{
				auto& i = item(n, false);

				if (cpies.empty())
					cpies.emplace_back();
				cpies.back().size += sizeof(i);

				n++;
				return i;
			}

			auto& add_item_overwrite_size(uint overwrite_size)
			{
				auto& i = item(n, false);

				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = n * sizeof(i);
					cpy.size = overwrite_size;
					cpies.push_back(cpy);
				}

				n++;
				return i;
			}

			void upload(CommandBuffer* cb)
			{
				StorageBuffer<T>::upload(cb);
				n = 0;
			}
		};
	}
}
