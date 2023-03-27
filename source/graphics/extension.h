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
				auto q = Queue::get();
				q->submit1(get(), nullptr, nullptr, fence);
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
				reset(Buffer::create(size, BufferUsageTransferSrc | BufferUsageTransferDst | extra_usage, MemoryPropertyHost | MemoryPropertyCoherent));
				get()->map();
				if (data)
					memcpy(get()->mapped, data, size);
			}
		};

		struct SparseArray
		{
			uint capacity;
			std::deque<uint> free_slots;

			void init(uint _capacity)
			{
				capacity = _capacity;
				free_slots.resize(capacity);
				std::iota(free_slots.begin(), free_slots.end(), 0);
			}

			inline int get_free_item()
			{
				if (free_slots.empty())
					return -1;
				auto ret = free_slots.front();
				free_slots.pop_front();
				return ret;
			}

			inline void release_item(uint id)
			{
				free_slots.push_back(id);
			}
		};

		constexpr inline AccessFlags u2a(BufferUsageFlags u)
		{
			if (u & BufferUsageVertex)
				return AccessVertexAttributeRead;
			if (u & BufferUsageIndex)
				return AccessIndexRead;
			if (u & BufferUsageIndirect)
				return AccessIndirectCommandRead;
			if (u & BufferUsageUniform)
				return AccessShaderRead;
			if (u & BufferUsageStorage)
				return AccessShaderRead | AccessShaderWrite;
			return AccessNone;
		}

		constexpr inline PipelineStageFlags u2s(BufferUsageFlags u)
		{
			if (u & BufferUsageVertex)
				return PipelineStageVertexInput;
			if (u & BufferUsageIndex)
				return PipelineStageVertexInput;
			if (u & BufferUsageIndirect)
				return PipelineStageDrawIndirect;
			return PipelineStageAllCommand;
		}

		struct DirtyMarkableVirtualObject : VirtualObject
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

		struct StorageBuffer : DirtyMarkableVirtualObject
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
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc | _stag_usage, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();

				VirtualObject::type = TypeInfo::get(TagU, ui->name, *ui->db);
				VirtualObject::create(stag->mapped);
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
					copies.push_back(cpy);
				}
				dirty_regions.clear();
				cb->copy_buffer(stag.get(), buf.get(), copies);
				cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(usage), PipelineStageTransfer, u2s(usage));
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
			std::unique_ptr<BufferT>	buf;
			std::unique_ptr<BufferT>	stag;
			uint						buf_top = 0;
			uint						stag_top = 0;
			TypeInfo*					item_type;

			void create(UdtInfo* ui, uint capacity)
			{
				buf.reset(Buffer::create(capacity * ui->size, BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();

				VirtualObject::type = TypeInfo::get(TagAU, std::format("{}[{}]", ui->name, capacity), *ui->db);
				VirtualObject::create(stag->mapped);
				item_type = VirtualObject::type->get_wrapped();
			}

			void create(const std::filesystem::path& vi_filename, const std::vector<std::string>& vi_defines, uint capacity)
			{
				create(get_vertex_input_ui(vi_filename, vi_defines), capacity);
			}

			void create(uint item_size, uint capacity)
			{
				buf.reset(Buffer::create(capacity * item_size, BufferUsageTransferDst | BufferUsageVertex, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();

				VirtualObject::type = TypeInfo::get(TagAD, std::format("Dummy_{}[{}]", item_size, capacity));
				VirtualObject::create(stag->mapped);
				item_type = VirtualObject::type->get_wrapped();
			}

			template<class T>
			T& item_t(int idx)
			{
				if (idx < 0)
					idx = stag_top + idx;
				return *(T*)(data + idx * item_type->size);
			}

			VirtualObject add()
			{
				return item(stag_top++);
			}

			template<class T>
			T& add_t()
			{
				auto& t = *(T*)(data + stag_top * item_type->size);
				stag_top++;
				return t;
			}

			void add(const void* src, uint size)
			{
				memcpy(data + stag_top * item_type->size, src, size * item_type->size);
				stag_top += size;
			}

			void upload(CommandBufferPtr cb)
			{
				if (buf_top < stag_top)
				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = buf_top * item_type->size;
					cpy.size = (stag_top - buf_top) * item_type->size;
					cb->copy_buffer(stag.get(), buf.get(), { &cpy, 1 });
					cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(BufferUsageVertex), PipelineStageTransfer, u2s(BufferUsageVertex));
					buf_top = stag_top;
				}
			}
		};

		template<typename T = uint>
		struct IndexBuffer
		{
			uint						capacity;
			std::unique_ptr<BufferT>	buf;
			std::unique_ptr<BufferT>	stag;
			uint						buf_top = 0;
			uint						stag_top = 0;

			void create(uint _capacity)
			{
				capacity = _capacity;
				buf.reset(Buffer::create(capacity * sizeof(T), BufferUsageTransferDst | BufferUsageIndex, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();
			}

			void add(T v)
			{
				memcpy((char*)stag->mapped + stag_top * sizeof(T), &v, sizeof(T));
				stag_top++;
			}

			void add(const T* src, uint size)
			{
				memcpy((char*)stag->mapped + stag_top * sizeof(T), src, size * sizeof(T));
				stag_top += size;
			}

			void upload(CommandBufferPtr cb)
			{
				if (buf_top < stag_top)
				{
					BufferCopy cpy;
					cpy.src_off = cpy.dst_off = buf_top * sizeof(T);
					cpy.size = (stag_top - buf_top) * sizeof(T);
					cb->copy_buffer(stag.get(), buf.get(), { &cpy, 1 });
					cb->buffer_barrier(buf.get(), AccessTransferWrite, u2a(BufferUsageIndex), PipelineStageTransfer, u2s(BufferUsageIndex));
					buf_top = stag_top;
				}
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
				buf.reset(Buffer::create(capacity * sizeof(graphics::DrawIndexedIndirectCommand), BufferUsageTransferDst | BufferUsageIndirect, MemoryPropertyDevice));
				stag.reset(Buffer::create(buf->size, BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				stag->map();
			}

			inline void add(uint index_count, uint first_index = 0, int vertex_offset = 0, uint instance_count = 1, uint first_instance = 0)
			{
				auto& c = *(DrawIndexedIndirectCommand*)((char*)stag->mapped + top * sizeof(graphics::DrawIndexedIndirectCommand));
				c.index_count = index_count;
				c.instance_count = instance_count;
				c.first_index = first_index;
				c.vertex_offset = vertex_offset;
				c.first_instance = first_instance;

				top++;
			}

			void upload(CommandBufferPtr cb)
			{
				if (top > 0)
				{
					BufferCopy cpy;
					cpy.size = top * sizeof(graphics::DrawIndexedIndirectCommand);
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

			DescriptorSetPtr					dss[8];
			DirtyMarkableVirtualObject			pc;

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

				memset(dss, 0, sizeof(dss));
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
					if (ii >= dsl_map.size() || ii >= countof(dss) || !dss[ii])
					{
						count = i;
						break;
					}
				}
				if (count > 0)
				{
					cb->bind_pipeline_layout(pll, plt);
					cb->bind_descriptor_sets(off, { dss + off, count });
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
