#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>

#if defined(FLAME_VULKAN)
#include <spirv_glsl.hpp>
#endif

#include <functional>

namespace flame
{
	namespace graphics
	{
		DescriptorpoolPrivate::DescriptorpoolPrivate(Device* _d) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorPoolSize descriptorPoolSizes[] = {
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 32},
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.poolSizeCount = FLAME_ARRAYSIZE(descriptorPoolSizes);
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 64;
			chk_res(vkCreateDescriptorPool(((DevicePrivate*)d)->v, &descriptorPoolInfo, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		DescriptorpoolPrivate::~DescriptorpoolPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorPool(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Descriptorpool* Descriptorpool::create(Device* d)
		{
			return new DescriptorpoolPrivate(d);
		}

		void Descriptorpool::destroy(Descriptorpool* p)
		{
			delete (DescriptorpoolPrivate*)p;
		}

		DescriptorsetlayoutPrivate::DescriptorsetlayoutPrivate(Device* _d, const std::vector<void*>& bindings) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			vk_bindings.resize(bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
			{
				auto bd = (DescriptorsetBinding*)bindings[i];

				vk_bindings[i].binding = bd->binding;
				vk_bindings[i].descriptorType = to_enum(bd->type);
				vk_bindings[i].descriptorCount = bd->count;
				vk_bindings[i].stageFlags = to_flags(ShaderStageAll);
				vk_bindings[i].pImmutableSamplers = nullptr;
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			chk_res(vkCreateDescriptorSetLayout(((DevicePrivate*)d)->v,
				&info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		DescriptorsetlayoutPrivate::~DescriptorsetlayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyDescriptorSetLayout(((DevicePrivate*)d)->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Descriptorsetlayout* Descriptorsetlayout::create(Device* d, const std::vector<void*>& bindings)
		{
			return new DescriptorsetlayoutPrivate(d, bindings);
		}

		void Descriptorsetlayout::destroy(Descriptorsetlayout* l)
		{
			delete (DescriptorsetlayoutPrivate*)l;
		}

		struct DescriptorsetBinding$
		{
			AttributeV<uint> binding$i;
			AttributeE<DescriptorType$> type$i;
			AttributeV<uint> count$i;

			AttributeV<DescriptorsetBinding> out$o;

			FLAME_GRAPHICS_EXPORTS DescriptorsetBinding$()
			{
				count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (binding$i.frame > out$o.frame)
					out$o.v.binding = binding$i.v;
				if (type$i.frame > out$o.frame)
					out$o.v.type = type$i.v;
				if (count$i.frame > out$o.frame)
					out$o.v.count = count$i.v;
				out$o.frame = maxN(binding$i.frame, type$i.frame, count$i.frame);
			}

		};

		struct Descriptorsetlayout$
		{
			AttributeP<std::vector<void*>> bindings$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (bindings$i.frame > out$o.frame)
				{
					if (out$o.v)
						Descriptorsetlayout::destroy((Descriptorsetlayout*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d)
						out$o.v = Descriptorsetlayout::create(d, bindings$i.v ? *bindings$i.v : std::vector<void*>());
					else
					{
						printf("cannot create descriptorsetlayout\n");

						out$o.v = nullptr;
					}
					out$o.frame = bindings$i.frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Descriptorsetlayout$()
			{
				if (out$o.v)
					Descriptorsetlayout::destroy((Descriptorsetlayout*)out$o.v);
			}

		};

		DescriptorsetPrivate::DescriptorsetPrivate(Descriptorpool* _p, Descriptorsetlayout* l) :
			p((DescriptorpoolPrivate*)_p)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = p->v;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &((DescriptorsetlayoutPrivate*)l)->v;

			chk_res(vkAllocateDescriptorSets(p->d->v, &info, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		DescriptorsetPrivate::~DescriptorsetPrivate()
		{
#if defined(FLAME_VULKAN)
			chk_res(vkFreeDescriptorSets(p->d->v, p->v, 1, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_uniformbuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorBufferInfo i;
			i.buffer = ((BufferPrivate*)b)->v;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_storagebuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorBufferInfo i;
			i.buffer = ((BufferPrivate*)b)->v;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_imageview(uint binding, uint index, Imageview* iv, Sampler* sampler)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorImageInfo i;
			i.imageView = ((ImageviewPrivate*)iv)->v;
			i.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			i.sampler = ((SamplerPrivate*)sampler)->v;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void DescriptorsetPrivate::set_storageimage(uint binding, uint index, Imageview* iv)
		{
#if defined(FLAME_VULKAN)
			VkDescriptorImageInfo i;
			i.imageView = ((ImageviewPrivate*)iv)->v;
			i.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			i.sampler = 0;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = v;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(p->d->v, 1, &write, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void Descriptorset::set_uniformbuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
			((DescriptorsetPrivate*)this)->set_uniformbuffer(binding, index, b, offset, range);
		}

		void Descriptorset::set_storagebuffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
			((DescriptorsetPrivate*)this)->set_storagebuffer(binding, index, b, offset, range);
		}

		void Descriptorset::set_imageview(uint binding, uint index, Imageview* v, Sampler* sampler)
		{
			((DescriptorsetPrivate*)this)->set_imageview(binding, index, v, sampler);
		}

		void Descriptorset::set_storageimage(uint binding, uint index, Imageview* v)
		{
			((DescriptorsetPrivate*)this)->set_storageimage(binding, index, v);
		}

		Descriptorset* Descriptorset::create(Descriptorpool* p, Descriptorsetlayout* l)
		{
			return new DescriptorsetPrivate(p, l);
		}

		void Descriptorset::destroy(Descriptorset* s)
		{
			delete (DescriptorsetPrivate*)s;
		}

		PipelinelayoutPrivate::PipelinelayoutPrivate(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, uint push_constant_udt_name_hash) :
			d((DevicePrivate*)d)
		{
			if (push_constant_udt_name_hash)
			{
				pc_udt = find_udt(push_constant_udt_name_hash);
				assert(pc_udt);
				pc_size = pc_udt->size();
			}
			else
			{
				pc_udt = nullptr;
				pc_size = push_constant_size;
			}

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayout> vk_descriptorsetlayouts;
			vk_descriptorsetlayouts.resize(descriptorsetlayouts.size());
			for (auto i = 0; i < vk_descriptorsetlayouts.size(); i++)
				vk_descriptorsetlayouts[i] = ((DescriptorsetlayoutPrivate*)descriptorsetlayouts[i])->v;

			VkPushConstantRange vk_pushconstant;
			vk_pushconstant.offset = 0;
			vk_pushconstant.size = pc_size;
			vk_pushconstant.stageFlags = to_flags(ShaderStageAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptorsetlayouts.size();
			info.pSetLayouts = vk_descriptorsetlayouts.data();
			info.pushConstantRangeCount = pc_size > 0 ? 1 : 0;
			info.pPushConstantRanges = pc_size > 0 ? &vk_pushconstant : nullptr;

			chk_res(vkCreatePipelineLayout(((DevicePrivate*)d)->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif


		}

		PipelinelayoutPrivate::~PipelinelayoutPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyPipelineLayout(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Pipelinelayout* Pipelinelayout::create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, uint push_constant_udt_name_hash)
		{
			return new PipelinelayoutPrivate(d, descriptorsetlayouts, push_constant_size, push_constant_udt_name_hash);
		}

		void Pipelinelayout::destroy(Pipelinelayout* l)
		{
			delete (PipelinelayoutPrivate*)l;
		}

		struct Pipelinelayout$
		{
			AttributeP<std::vector<void*>> descriptorsetlayouts$i;
			AttributeV<uint> push_constant_size$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (descriptorsetlayouts$i.frame > out$o.frame || push_constant_size$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipelinelayout::destroy((Pipelinelayout*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d && descriptorsetlayouts$i.v && !descriptorsetlayouts$i.v->empty())
						out$o.v = Pipelinelayout::create(d, *descriptorsetlayouts$i.v, push_constant_size$i.v);
					else
					{
						printf("cannot create pipelinelayout\n");

						out$o.v = nullptr;
					}
					out$o.frame = max(descriptorsetlayouts$i.frame, push_constant_size$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Pipelinelayout$()
			{
				if (out$o.v)
					Pipelinelayout::destroy((Pipelinelayout*)out$o.v);
			}

		};

		ShaderPrivate::ShaderPrivate(Device* d, const std::wstring& filename, const std::string& _prefix, const std::vector<void*>& _inputs, const std::vector<void*>& _outputs, Pipelinelayout* _pll, bool autogen_code) :
			d((DevicePrivate*)d)
		{
			assert(std::fs::exists(filename));

			auto ext = std::fs::path(filename).extension();
			if (ext == L".vert")
				stage = ShaderStageVert;
			else if (ext == L".tesc")
				stage = ShaderStageTesc;
			else if (ext == L".tese")
				stage = ShaderStageTese;
			else if (ext == L".geom")
				stage = ShaderStageGeom;
			else if (ext == L".frag")
				stage = ShaderStageFrag;
			else if (ext == L".comp")
				stage = ShaderStageComp;

			auto pll = (PipelinelayoutPrivate*)_pll;

			auto prefix = _prefix;
			if (autogen_code)
			{
				if (pll)
				{
					auto udt = pll->pc_udt;
					if (udt)
					{
						prefix += "\nlayout(push_constant) uniform PushconstantT\n{\n";
						for (auto i = 0; i < udt->variable_count(); i++)
						{
							auto v = udt->variable(i);
							auto t = v->type();
							assert(t->tag() == TypeTagVariable);
							std::string type_name;
							switch (t->hash())
							{
							case cH("int"):
								type_name = "int";
								break;
							case cH("Vec(2+int)"):
								type_name = "ivec2";
								break;
							case cH("Vec(3+int)"):
								type_name = "ivec3";
								break;
							case cH("Vec(4+int)"):
								type_name = "ivec4";
								break;
							case cH("float"):
								type_name = "float";
								break;
							case cH("Vec(2+float)"):
								type_name = "vec2";
								break;
							case cH("Vec(3+float)"):
								type_name = "vec3";
								break;
							case cH("Vec(4+float)"):
								type_name = "vec4";
								break;
							default:
								assert(0);
							}
							prefix += "\t" + type_name + " " + v->name() + ";\n";
						}
						prefix += "}pc;\n";
					}
				}
			}
			
			auto hash = H(prefix.c_str());
			std::wstring spv_filename(filename + L"." + std::to_wstring(hash) + L".spv");

			if (!std::fs::exists(spv_filename) || std::fs::last_write_time(spv_filename) < std::fs::last_write_time(filename))
			{
				auto vk_sdk_path = s2w(getenv("VK_SDK_PATH"));
				assert(vk_sdk_path != L"");

				std::fs::remove(spv_filename);

				auto temp_filename = L"temp" + ext.wstring();
				{
					std::ofstream ofile(temp_filename);
					auto file = get_file_string(filename);
					ofile << "#version 450 core\n";
					ofile << "#extension GL_ARB_shading_language_420pack : enable\n";
					if (stage != ShaderStageComp)
						ofile << "#extension GL_ARB_separate_shader_objects : enable\n";
					ofile << "\n" << prefix << "\n";
					ofile << file;
					ofile.close();
				}
				std::wstring command_line(L" " + temp_filename + L" -flimit-file shader.conf -o" + spv_filename);
				auto output = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
				std::fs::remove(temp_filename);
				if (!std::fs::exists(spv_filename))
					printf("shader \"%s\" compile error:\n%s\n", w2s(filename).c_str(), output.p->c_str());
				delete_mail(output);
			}

			auto spv_file = get_file_content(spv_filename);
			if (!spv_file.first)
				assert(0);

#if defined(FLAME_VULKAN)
			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = spv_file.second;
			shader_info.pCode = (uint*)spv_file.first.get();
			chk_res(vkCreateShaderModule(((DevicePrivate*)d)->v, &shader_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
			{
				spirv_cross::CompilerGLSL compiler((uint*)spv_file.first.get(), spv_file.second / sizeof(uint));
				auto resources = compiler.get_shader_resources();

				std::function<void(uint, Variable*)> get_v;
				get_v = [&](uint type_id, Variable* v) {
					const auto* t = &compiler.get_type(type_id);
					while (t->pointer)
					{
						type_id = t->parent_type;
						t = &compiler.get_type(type_id);
					}

					assert(t->array.size() <= 1); // multidimensional array is WIP
					v->count = t->array.empty() ? 1 : t->array[0];
					if (t->array.empty())
						v->array_stride = 0;
					else
						v->array_stride = compiler.get_decoration(type_id, spv::DecorationArrayStride);

					if (t->basetype == spirv_cross::SPIRType::Struct)
					{
						v->type_name = compiler.get_name(type_id);
						v->size = compiler.get_declared_struct_size(*t);
						for (auto i = 0; i < t->member_types.size(); i++)
						{
							auto m = new Variable;
							m->name = compiler.get_member_name(type_id, i);
							m->offset = compiler.type_struct_member_offset(*t, i);
							m->size = compiler.get_declared_struct_member_size(*t, i);
							v->members.emplace_back(m);
							get_v(t->member_types[i], m);
						}
					}
					else
					{
						std::string base_name;
						switch (t->basetype)
						{
						case spirv_cross::SPIRType::SByte:
							base_name = "char";
							break;
						case spirv_cross::SPIRType::UByte:
							base_name = "uchar";
							break;
						case spirv_cross::SPIRType::Short:
							base_name = "short";
							break;
						case spirv_cross::SPIRType::UShort:
							base_name = "ushort";
							break;
						case spirv_cross::SPIRType::Int:
							base_name = "int";
							break;
						case spirv_cross::SPIRType::UInt:
							base_name = "uint";
							break;
						case spirv_cross::SPIRType::Float:
							base_name = "float";
							break;
						default:
							assert(0);
						}
						if (t->columns <= 1)
						{
							if (t->vecsize <= 1)
								v->type_name = base_name;
							else
								v->type_name = "Vec(" + std::to_string(t->vecsize) + "+" + base_name + ")";
						}
						else
							v->type_name = "Mat(" + std::to_string(t->vecsize) + "+" + std::to_string(t->columns) + "+" + base_name + ")";
					}
				};

				if (stage == ShaderStageVert)
				{
					for (auto& src : resources.stage_inputs)
					{
						auto r = new Resource;
						r->location = compiler.get_decoration(src.id, spv::DecorationLocation);
						r->name = src.name;
						get_v(src.type_id, &r->v);
						vias.emplace_back(r);
					}
				}

				assert(resources.push_constant_buffers.size() <= 1);
				if (!resources.push_constant_buffers.empty())
				{
					auto& src = resources.push_constant_buffers[0];
					pc.reset(new Resource);
					pc->name = src.name;

					get_v(src.type_id, &pc->v);
				}

				if (pll)
				{
					if (pc)
					{
						auto pcv = &pc->v;
						assert(!pc || pcv->size == pll->pc_size);
						auto udt = pll->pc_udt;
						if (udt)
						{
							assert(pcv->members.size() == udt->variable_count());
							for (auto i = 0; i < pcv->members.size(); i++)
							{
								auto m = pcv->members[i].get();
								assert(m->members.empty()); // nested structs are WIP

								auto v = udt->variable(i);
								assert(m->type_name == v->type()->name());
								assert(m->name == v->name());
								assert(m->offset == v->offset());
								assert(m->size == v->size());
								assert(m->count == 1); // count is WIP
							}
						}
					}
				}
			}
		}

		ShaderPrivate::~ShaderPrivate()
		{
#if defined(FLAME_VULKAN)
			if (v)
				vkDestroyShaderModule(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Shader* Shader::create(Device* d, const std::wstring& filename, const std::string& prefix, const std::vector<void*>& inputs, const std::vector<void*>& outputs, Pipelinelayout* pll, bool autogen_code)
		{
			return new ShaderPrivate(d, filename, prefix, inputs, outputs, pll, autogen_code);
		}

		void Shader::destroy(Shader* s)
		{
			delete (ShaderPrivate*)s;
		}

		struct Shader$
		{
			AttributeV<std::wstring> filename$i;
			AttributeV<std::string> prefix$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Shader$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (out$o.frame || filename$i.frame > out$o.frame || prefix$i.frame > out$o.frame)
				{
					if (out$o.v)
						Shader::destroy((Shader*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d)
						out$o.v = Shader::create(d, bp_environment().path + L"/" + filename$i.v, prefix$i.v);
					else
					{
						printf("cannot create shader\n");

						out$o.v = nullptr;
					}
					out$o.frame = max(filename$i.frame, prefix$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Shader$()
			{
				if (out$o.v)
					Shader::destroy((Shader*)out$o.v);
			}

		};

		PipelinePrivate::PipelinePrivate(Device* _d, const std::vector<void*>& shaders, Pipelinelayout* _pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* _vi, const Vec2u& _vp, RasterInfo* _raster, SampleCount$ _sc, DepthInfo* _depth, const std::vector<void*>& _output_states, const std::vector<uint>& _dynamic_states) :
			d((DevicePrivate*)_d),
			pll((PipelinelayoutPrivate*)_pll)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			vk_stage_infos.resize(shaders.size());
			for (auto i = 0; i < shaders.size(); i++)
			{
				auto src = (ShaderPrivate*)shaders[i];
				auto& dst = vk_stage_infos[i];
				dst.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				dst.flags = 0;
				dst.pNext = nullptr;
				dst.pSpecializationInfo = nullptr;
				dst.pName = "main";
				dst.stage = to_enum(src->stage);
				dst.module = src->v;
			}

			if (_vi)
			{
				vk_vi_attributes.resize(_vi->attribs.size());
				for (auto i = 0; i < _vi->attribs.size(); i++)
				{
					const auto& src = *(VertexInputAttributeInfo*)_vi->attribs[i];
					auto& dst = vk_vi_attributes[i];
					dst.location = src.location;
					dst.binding = src.buffer_id;
					dst.offset = src.offset;
					dst.format = to_enum(src.format);
				}
				vk_vi_bindings.resize(_vi->buffers.size());
				for (auto i = 0; i < _vi->buffers.size(); i++)
				{
					const auto& src = *(VertexInputBufferInfo*)_vi->buffers[i];
					auto& dst = vk_vi_bindings[i];
					dst.binding = src.id;
					dst.stride = src.stride;
					dst.inputRate = to_enum(src.rate);
				}
			}

			VkPipelineVertexInputStateCreateInfo vertex_input_state;
			vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertex_input_state.pNext = nullptr;
			vertex_input_state.flags = 0;
			vertex_input_state.vertexBindingDescriptionCount = vk_vi_bindings.size();
			vertex_input_state.pVertexBindingDescriptions = vk_vi_bindings.empty() ? nullptr : vk_vi_bindings.data();
			vertex_input_state.vertexAttributeDescriptionCount = vk_vi_attributes.size();
			vertex_input_state.pVertexAttributeDescriptions = vk_vi_attributes.empty() ? nullptr : vk_vi_attributes.data();

			VkPipelineInputAssemblyStateCreateInfo assembly_state;
			assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			assembly_state.flags = 0;
			assembly_state.pNext = nullptr;
			assembly_state.topology = to_enum(_vi ? _vi->primitive_topology : PrimitiveTopologyTriangleList);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state;
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.pNext = nullptr;
			tess_state.flags = 0;
			tess_state.patchControlPoints = _vi ? _vi->patch_control_points : 0;

			VkViewport viewport;
			viewport.width = (float)_vp.x();
			viewport.height = (float)_vp.y();
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			viewport.x = 0;
			viewport.y = 0;

			VkRect2D scissor;
			scissor.extent.width = _vp.x();
			scissor.extent.height = _vp.y();
			scissor.offset.x = 0;
			scissor.offset.y = 0;

			VkPipelineViewportStateCreateInfo viewport_state;
			viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewport_state.pNext = nullptr;
			viewport_state.flags = 0;
			viewport_state.viewportCount = 1;
			viewport_state.scissorCount = 1;
			viewport_state.pScissors = &scissor;
			viewport_state.pViewports = &viewport;

			VkPipelineRasterizationStateCreateInfo raster_state;
			raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			raster_state.pNext = nullptr;
			raster_state.flags = 0;
			raster_state.depthClampEnable = _raster ? _raster->depth_clamp : false;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.polygonMode = to_enum(_raster ? _raster->polygon_mode : PolygonModeFill);
			raster_state.cullMode = to_enum(_raster ? _raster->cull_mode : CullModeBack);
			raster_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			raster_state.depthBiasEnable = VK_FALSE;
			raster_state.depthBiasConstantFactor = 0.f;
			raster_state.depthBiasClamp = 0.f;
			raster_state.depthBiasSlopeFactor = 0.f;
			raster_state.lineWidth = 1.f;

			VkPipelineMultisampleStateCreateInfo multisample_state;
			multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisample_state.flags = 0;
			multisample_state.pNext = nullptr;
			multisample_state.rasterizationSamples = to_enum(_sc);
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.minSampleShading = 0.f;
			multisample_state.pSampleMask = nullptr;
			multisample_state.alphaToCoverageEnable = VK_FALSE;
			multisample_state.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = _depth ? _depth->test : false;
			depth_stencil_state.depthWriteEnable = _depth ? _depth->write : false;
			depth_stencil_state.depthCompareOp = to_enum(_depth ? _depth->compare_op : CompareOpLess);
			depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			depth_stencil_state.minDepthBounds = 0;
			depth_stencil_state.maxDepthBounds = 0;
			depth_stencil_state.stencilTestEnable = VK_FALSE;
			depth_stencil_state.front = {};
			depth_stencil_state.back = {};

			vk_blend_attachment_states.resize(rp->color_attachment_count());
			for (auto i = 0; i < _output_states.size(); i++)
			{
				const auto& src = *(OutputAttachmentInfo*)_output_states[i];
				auto& dst = vk_blend_attachment_states[i];
				dst.blendEnable = src.blend_enable;
				dst.srcColorBlendFactor = to_enum(src.blend_src_color);
				dst.dstColorBlendFactor = to_enum(src.blend_dst_color);
				dst.colorBlendOp = VK_BLEND_OP_ADD;
				dst.srcAlphaBlendFactor = to_enum(src.blend_src_alpha);
				dst.dstAlphaBlendFactor = to_enum(src.blend_dst_alpha);
				dst.alphaBlendOp = VK_BLEND_OP_ADD;
				dst.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}

			VkPipelineColorBlendStateCreateInfo blend_state;
			blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blend_state.flags = 0;
			blend_state.pNext = nullptr;
			blend_state.blendConstants[0] = 0.f;
			blend_state.blendConstants[1] = 0.f;
			blend_state.blendConstants[2] = 0.f;
			blend_state.blendConstants[3] = 0.f;
			blend_state.logicOpEnable = VK_FALSE;
			blend_state.logicOp = VK_LOGIC_OP_COPY;
			blend_state.attachmentCount = vk_blend_attachment_states.size();
			blend_state.pAttachments = vk_blend_attachment_states.data();

			for (auto& s : _dynamic_states)
				vk_dynamic_states.push_back(to_enum((DynamicState)s));
			if (_vp.x() == 0 && _vp.y() == 0)
			{
				if (std::find(vk_dynamic_states.begin(), vk_dynamic_states.end(), VK_DYNAMIC_STATE_VIEWPORT) == vk_dynamic_states.end())
					vk_dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
				if (std::find(vk_dynamic_states.begin(), vk_dynamic_states.end(), VK_DYNAMIC_STATE_SCISSOR) == vk_dynamic_states.end())
					vk_dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);
			}

			VkPipelineDynamicStateCreateInfo dynamic_state;
			dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamic_state.pNext = nullptr;
			dynamic_state.flags = 0;
			dynamic_state.dynamicStateCount = vk_dynamic_states.size();
			dynamic_state.pDynamicStates = vk_dynamic_states.data();

			VkGraphicsPipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipeline_info.pNext = nullptr;
			pipeline_info.flags = 0;
			pipeline_info.stageCount = vk_stage_infos.size();
			pipeline_info.pStages = vk_stage_infos.data();
			pipeline_info.pVertexInputState = &vertex_input_state;
			pipeline_info.pInputAssemblyState = &assembly_state;
			pipeline_info.pTessellationState = tess_state.patchControlPoints > 0 ? &tess_state : nullptr;
			pipeline_info.pViewportState = &viewport_state;
			pipeline_info.pRasterizationState = &raster_state;
			pipeline_info.pMultisampleState = &multisample_state;
			pipeline_info.pDepthStencilState = &depth_stencil_state;
			pipeline_info.pColorBlendState = &blend_state;
			pipeline_info.pDynamicState = vk_dynamic_states.size() ? &dynamic_state : nullptr;
			pipeline_info.layout = pll->v;
			pipeline_info.renderPass = rp ? ((RenderpassPrivate*)rp)->v : nullptr;
			pipeline_info.subpass = subpass_idx;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineGraphics;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, Shader* compute_shader, Pipelinelayout* _pll) :
			d((DevicePrivate*)_d),
			pll((PipelinelayoutPrivate*)_pll)
		{
#if defined(FLAME_VULKAN)

			VkComputePipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipeline_info.pNext = nullptr;
			pipeline_info.flags = 0;

			pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipeline_info.stage.flags = 0;
			pipeline_info.stage.pNext = nullptr;
			pipeline_info.stage.pSpecializationInfo = nullptr;
			pipeline_info.stage.pName = "main";
			pipeline_info.stage.stage = to_enum(ShaderStageComp);
			pipeline_info.stage.module = ((ShaderPrivate*)compute_shader)->v;

			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = pll->v;

			chk_res(vkCreateComputePipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineCompute;
		}

		PipelinePrivate::~PipelinePrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyPipeline(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Pipeline* Pipeline::create(Device* d, const std::vector<void*>& shaders, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, const std::vector<void*>& output_states, const std::vector<uint>& dynamic_states)
		{
			return new PipelinePrivate(d, shaders, pll, rp, subpass_idx, vi, vp, raster, sc, depth, output_states, dynamic_states);
		}

		Pipeline* Pipeline::create(Device* d, Shader* compute_shader, Pipelinelayout* pll)
		{
			return new PipelinePrivate(d, compute_shader, pll);
		}

		void Pipeline::destroy(Pipeline* p)
		{
			delete (PipelinePrivate*)p;
		}

		struct PipelineFullscreen$
		{
			AttributeP<void> renderpass$i;
			AttributeV<uint> subpass_idx$i;
			AttributeP<std::vector<void*>> shaders$i;
			AttributeP<void> layout$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (renderpass$i.frame > out$o.frame || subpass_idx$i.frame > out$o.frame || shaders$i.frame > out$o.frame ||layout$i.frame > out$o.frame)
				{
					if (out$o.v)
						Pipeline::destroy((Pipeline*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d && renderpass$i.v && ((Renderpass*)renderpass$i.v)->subpass_count() > subpass_idx$i.v && shaders$i.v && !shaders$i.v->empty() && layout$i.v)
						out$o.v = Pipeline::create(d, *shaders$i.v, (Pipelinelayout*)layout$i.v, (Renderpass*)renderpass$i.v, subpass_idx$i.v);
					else
					{
						printf("cannot create pipelinefullscreen\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(renderpass$i.frame, subpass_idx$i.frame, shaders$i.frame, layout$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~PipelineFullscreen$()
			{
				if (out$o.v)
					Pipeline::destroy((Pipeline*)out$o.v);
			}

		};
	}
}
