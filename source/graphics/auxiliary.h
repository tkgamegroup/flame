#pragma once

#include "../foundation/typeinfo.h"
#include "command.h"
#include "buffer.h"
#include "shader.h"

namespace flame
{
	namespace graphics
	{
		struct InstanceCommandBuffer : std::unique_ptr<CommandBufferT>
		{
			FencePtr fence;

			InstanceCommandBuffer(FencePtr fence = nullptr) :
				fence(fence)
			{
				reset(CommandBuffer::create(CommandPool::get()));
				get()->begin(true);
			}

			void excute()
			{
				get()->end();
				auto q = (Queue*)Queue::get();
				q->submit(get(), fence);
				if (!fence)
					q->wait_idle();
				else
					fence->wait();
			}
		};

		struct StagingBuffer : std::unique_ptr<BufferT>
		{
			StagingBuffer(uint size, void* data = nullptr, BufferUsageFlags extra_usage = BufferUsageNone)
			{
				reset(Buffer::create(size, BufferUsageTransferSrc | extra_usage, MemoryPropertyHost | MemoryPropertyCoherent));
				get()->map();
				if (data)
					memcpy(get()->mapped, data, size);
			}
		};

		struct VirtualObjectWithDirtyRegions : VirtualObject
		{
			std::vector<std::pair<uint, uint>>	dirty_regions;

			void mark_dirty()
			{
				mark_dirty(0, type->size);
			}

			void mark_dirty(const VirtualObject& vo)
			{
				mark_dirty(offset(vo), vo.type->size);
			}

			void mark_dirty(uint off, uint sz)
			{
				if (sz == 0)
					return;
				if (!dirty_regions.empty())
				{
					auto& last = dirty_regions.back();
					if (last.first + last.second == off)
					{
						last.second += sz;
						return;
					}
				}
				dirty_regions.emplace_back(off, sz);
			}

			// get child and mark it dirty
			VirtualObject mark_dirty_c(uint hash)
			{
				auto vo = child(hash);
				mark_dirty(vo);
				return vo;
			}

			// get child array item and mark it diray
			VirtualObject mark_dirty_ci(uint hash, uint idx)
			{
				auto vo = child(hash).item(idx);
				mark_dirty(vo);
				return vo;
			}
		};

		struct StorageBuffer : VirtualObjectWithDirtyRegions
		{
			BufferUsageFlags					usage;
			std::unique_ptr<BufferT>			buf;
			std::unique_ptr<BufferT>			stag;

			StorageBuffer()
			{
			}

			StorageBuffer(BufferUsageFlags _usage, UdtInfo* ui, BufferUsageFlags _stag_usage = BufferUsageNone)
			{
				create(_usage, ui, _stag_usage);
			}

			void create(BufferUsageFlags _usage, UdtInfo* ui, BufferUsageFlags _stag_usage = BufferUsageNone)
			{
				usage = _usage;
				buf.reset(Buffer::create(ui->size, BufferUsageTransferDst | usage, MemoryPropertyDevice));
				buf->ui = ui;
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc | _stag_usage, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->ui = ui;
				stag->map();

				VirtualObject::type = TypeInfo::get(TagU, ui->name, *ui->db);
				VirtualObject::create(stag->mapped);
			}

			void upload()
			{
				graphics::InstanceCommandBuffer cb;
				upload(cb.get());
				cb.excute();
			}

			void upload(CommandBufferPtr cb)
			{
				if (dirty_regions.empty())
					return;
				std::vector<BufferCopy> copies;
				for (auto& r : dirty_regions)
				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = r.first;
					cpy.size = r.second;
					if (cpy.size > 0)
						copies.push_back(cpy);
				}
				dirty_regions.clear();
				if (!copies.empty())
					cb->copy_buffer(stag.get(), buf.get(), copies);
				cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(usage), PipelineStageTransfer, u2s(usage));
			}

			void download()
			{
				graphics::InstanceCommandBuffer cb;
				download(cb.get());
				cb.excute();
			}

			void download(CommandBufferPtr cb)
			{
				if (dirty_regions.empty())
					return;
				std::vector<BufferCopy> copies;
				for (auto& r : dirty_regions)
				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = r.first;
					cpy.size = r.second;
					copies.push_back(cpy);
				}
				dirty_regions.clear();
				cb->buffer_barrier(buf.get(), u2a(usage), AccessTransferWrite, u2s(usage), PipelineStageTransfer);
				cb->copy_buffer(buf.get(), stag.get(), copies);
			}
		};

		struct VertexBuffer : VirtualObject
		{
			uint 								capacity;
			std::unique_ptr<BufferT>			buf;
			std::unique_ptr<BufferT>			stag;
			TypeInfo*							item_type;
			SparseRanges						sparse_ranges;
			std::vector<std::pair<uint, uint>>	dirty_regions;

			void reset()
			{
				sparse_ranges.init(capacity);
			}

			void create(UdtInfo* ui, uint _capacity)
			{
				capacity = _capacity;
				buf.reset(Buffer::create(capacity * ui->size, BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();

				sparse_ranges.init(capacity);

				VirtualObject::type = TypeInfo::get(TagAU, std::format("{}[{}]", ui->name, capacity), *ui->db);
				VirtualObject::create(stag->mapped);
				item_type = VirtualObject::type->get_wrapped();

				reset();
			}

			void create(const std::filesystem::path& vi_filename, const std::vector<std::string>& vi_defines, uint capacity)
			{
				create(get_vertex_input_ui(vi_filename, vi_defines), capacity);
			}

			void create(uint item_size, uint _capacity)
			{
				capacity = _capacity;
				buf.reset(Buffer::create(capacity * item_size, BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();

				VirtualObject::type = TypeInfo::get(TagAD, std::format("Dummy_{}[{}]", item_size, capacity));
				VirtualObject::create(stag->mapped);
				item_type = VirtualObject::type->get_wrapped();

				reset();
			}

			template<class T>
			T& item_t(uint idx)
			{
				return *(T*)(data + idx * item_type->size);
			}

			int add(const void* src, uint size)
			{
				int off = sparse_ranges.get_free_space(size);
				if (off != -1)
				{
					uint data_off = off * item_type->size;
					uint data_size = size * item_type->size;
					if (src)
						memcpy(data + data_off, src, data_size);
					if (!dirty_regions.empty())
					{
						auto& last = dirty_regions.back();
						if (data_off >= last.first)
						{
							if (data_off + data_size <= last.first + last.second)
								return off;
							last.second = data_off + data_size - last.first;
							return off;
						}
					}
					dirty_regions.emplace_back(data_off, data_size);
				}
				else
					printf("Vertex buffer add: exceeded capacity\n");
				return off;
			}

			void release(uint off, uint size)
			{
				sparse_ranges.release_space(off, size);
			}

			void upload(CommandBufferPtr cb)
			{
				if (dirty_regions.empty())
					return;
				std::vector<BufferCopy> copies;
				for (auto& r : dirty_regions)
				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = r.first;
					cpy.size = r.second;
					if (cpy.size > 0)
						copies.push_back(cpy);
				}
				dirty_regions.clear();
				cb->buffer_barrier(buf.get(), u2a(BufferUsageVertex), AccessTransferWrite, u2s(BufferUsageVertex), PipelineStageTransfer);
				if (!copies.empty())
					cb->copy_buffer(stag.get(), buf.get(), copies);
				cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(BufferUsageVertex), PipelineStageTransfer, u2s(BufferUsageVertex));
			}
		};

		struct PushBuffer
		{
			uint						item_size;
			uint 						capacity;
			std::unique_ptr<BufferT>	buf;
			char*						dst = nullptr;
			uint						top = 0;

			void create(uint _item_size, uint _capacity, BufferUsageFlags usage)
			{
				item_size = _item_size;
				capacity = _capacity;
				buf.reset(Buffer::create(capacity * item_size, usage, MemoryPropertyHost | MemoryPropertyCoherent));
				buf->map();
				dst = (char*)buf->mapped;
			}

			void reset()
			{
				top = 0;
			}

			void add(uint n, void* data)
			{
				memcpy(dst + item_size * top, data, n * item_size);
				top += n;
			}

			template<class T>
			void add(const T& v)
			{
				memcpy(dst + item_size * top, &v, item_size);
				top += 1;
			}

			template<class T>
			T* pitem(uint idx)
			{
				return (T*)(dst + idx * item_size);
			}

			template<class T>
			T& item(uint idx)
			{
				return *(T*)(dst + idx * item_size);
			}
		};

		template<typename T = uint>
		struct IndexBuffer
		{
			uint								capacity;
			std::unique_ptr<BufferT>			buf;
			std::unique_ptr<BufferT>			stag;
			SparseRanges						sparse_ranges;
			std::vector<std::pair<uint, uint>>	dirty_regions;

			void reset()
			{
				sparse_ranges.init(capacity);
			}

			void create(uint _capacity)
			{
				capacity = _capacity;
				buf.reset(Buffer::create(capacity * sizeof(T), BufferUsageTransferDst | BufferUsageIndex, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();

				reset();
			}

			int add(const T* src, uint size)
			{
				int off = sparse_ranges.get_free_space(size);
				if (off != -1)
				{
					uint data_off = off * sizeof(T);
					uint data_size = size * sizeof(T);
					if (src)
						memcpy((char*)stag->mapped + data_off, src, data_size);
					if (!dirty_regions.empty())
					{
						auto& last = dirty_regions.back();
						if (data_off >= last.first)
						{
							if (data_off + data_size <= last.first + last.second)
								return off;
							last.second = data_off + data_size - last.first;
							return off;
						}
					}
					dirty_regions.emplace_back(data_off, data_size);
				}
				return off;
			}

			void release(uint off, uint size)
			{
				sparse_ranges.release_space(off, size);
			}

			void upload(CommandBufferPtr cb)
			{
				if (dirty_regions.empty())
					return;
				std::vector<BufferCopy> copies;
				for (auto& r : dirty_regions)
				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = r.first;
					cpy.size = r.second;
					if (cpy.size > 0)
						copies.push_back(cpy);
				}
				dirty_regions.clear();
				cb->buffer_barrier(buf.get(), u2a(BufferUsageIndex), AccessTransferWrite, u2s(BufferUsageIndex), PipelineStageTransfer);
				if (!copies.empty())
					cb->copy_buffer(stag.get(), buf.get(), copies);
				cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(BufferUsageIndex), PipelineStageTransfer, u2s(BufferUsageIndex));
			}
		};

		struct IndirectBuffer
		{
			uint						capacity;
			std::unique_ptr<BufferT>	buf;
			std::unique_ptr<BufferT>	stag;
			uint						top = 0;

			void create(uint _capacity)
			{
				capacity = _capacity;
				buf.reset(Buffer::create(capacity * sizeof(DrawIndexedIndirectCommand), BufferUsageTransferDst | BufferUsageIndirect, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();
			}

			inline void add(uint index_count, uint first_index = 0, int vertex_offset = 0, uint instance_count = 1, uint first_instance = 0)
			{
				auto c = (DrawIndexedIndirectCommand*)((char*)buf->mapped + top * sizeof(DrawIndexedIndirectCommand));
				if (top > 0)
				{
					c--;
					if (c->index_count == index_count && c->first_index == first_index && c->vertex_offset == vertex_offset
						&& c->first_instance + c->instance_count == first_instance)
					{
						c->instance_count += instance_count;
						return;
					}
					c++;
				}

				c->index_count = index_count;
				c->instance_count = instance_count;
				c->first_index = first_index;
				c->vertex_offset = vertex_offset;
				c->first_instance = first_instance;
				top++;
			}

			void upload(CommandBufferPtr cb)
			{
				if (top > 0)
				{
					BufferCopy cpy;
					cpy.size = top * sizeof(DrawIndexedIndirectCommand);
					cb->copy_buffer(stag.get(), buf.get(), { &cpy, 1 });
					cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(BufferUsageIndirect), PipelineStageTransfer, u2s(BufferUsageIndirect));
					top = 0;
				}
			}
		};

		struct PipelineResourceManager
		{
			PipelineLayoutPtr					pll = nullptr;
			PipelineType						plt = PipelineGraphics;
			std::unordered_map<uint, int>		dsl_map;

			std::vector<DescriptorSetPtr>		dss;
			VirtualObjectWithDirtyRegions		pc;

			PipelineResourceManager()
			{
			}

			PipelineResourceManager(PipelineLayoutPtr _pll, PipelineType _plt)
			{
				init(_pll, _plt);
			}

			~PipelineResourceManager()
			{
				if (pc.data)
					pc.destroy();
			}

			void init(PipelineLayoutPtr _pll, PipelineType _plt)
			{
				pll = _pll;
				plt = _plt;
				pll->ref++;

				for (auto i = 0; i < pll->dsls.size(); i++)
				{
					auto dsl = pll->dsls[i];

					std::string name;
					if (dsl->filename != pll->filename)
						name = dsl->filename.stem().string();
					else
						name = "";
					dsl_map.emplace(sh(name.c_str()), i);
				}

				auto ui = pll->pc_ui;
				if (ui)
				{
					pc.type = TypeInfo::get(TagU, ui->name, *ui->db);
					pc.create();
				}

				dss.resize(pll->dsls.size());
			}

			inline int dsl_idx(uint nh)
			{
				auto it = dsl_map.find(nh);
				if (it == dsl_map.end())
				{
					assert(0);
					return -1;
				}
				return it->second;
			}

			inline DescriptorSetLayoutPtr get_dsl(uint nh)
			{
				auto idx = dsl_idx(nh);
				return idx == -1 ? nullptr : pll->dsls[idx];
			}

			inline void set_ds(uint nh, DescriptorSetPtr ds)
			{
				auto idx = dsl_idx(nh);
				if (idx != -1)
					dss[idx] = ds;
			}

			inline void bind_dss(CommandBufferPtr cb, uint off = 0, uint count = 0xffffffff)
			{
				for (auto i = 0U; i < count; i++)
				{
					auto ii = off + i;
					if (ii >= dss.size() || ii >= dss.size() || !dss[ii])
					{
						count = i;
						break;
					}
				}
				if (count > 0)
				{
					cb->bind_pipeline_layout(pll, plt);
					cb->bind_descriptor_sets(off, { dss.data() + off, count});
				}
			}

			inline void push_constant(CommandBufferPtr cb)
			{
				cb->bind_pipeline_layout(pll);
				if (!pc.dirty_regions.empty())
				{
					for (auto& r : pc.dirty_regions)
						cb->push_constant(r.first, r.second, pc.data + r.first);
					pc.dirty_regions.clear();
				}
				else
					cb->push_constant(0, pll->pc_sz, pc.data);
			}
		};
	}
}
