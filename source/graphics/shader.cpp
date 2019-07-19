#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>

#if defined(FLAME_VULKAN)
#include <spirv_glsl.hpp>
#endif

namespace flame
{
	namespace graphics
	{
		DescriptorpoolPrivate::DescriptorpoolPrivate(Device* _d)
		{
			d = (DevicePrivate*)_d;

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

		DescriptorsetlayoutPrivate::DescriptorsetlayoutPrivate(Device* _d, const std::vector<Binding>& _bindings)
		{
			bindings = _bindings;

			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			vk_bindings.resize(bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
			{
				vk_bindings[i].binding = bindings[i].binding;
				vk_bindings[i].descriptorType = to_enum(bindings[i].type);
				vk_bindings[i].descriptorCount = bindings[i].count;
				vk_bindings[i].stageFlags = to_flags(ShaderAll);
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

		Descriptorsetlayout* Descriptorsetlayout::create(Device* d, const std::vector<Binding>& bindings)
		{
			return new DescriptorsetlayoutPrivate(d, bindings);
		}

		void Descriptorsetlayout::destroy(Descriptorsetlayout* l)
		{
			delete (DescriptorsetlayoutPrivate*)l;
		}

		DescriptorsetPrivate::DescriptorsetPrivate(Descriptorpool* _p, Descriptorsetlayout* l)
		{
			p = (DescriptorpoolPrivate*)_p;

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

		static std::wstring shader_path(L"../shader/");
		static std::wstring conf_path(shader_path + L"src/config.conf");

#if defined(FLAME_VULKAN)
		static void serialize_members(spirv_cross::CompilerGLSL &glsl, uint32_t tid, SerializableNode *dst)
		{
			auto t = glsl.get_type(tid);
			auto cnt = t.member_types.size();
			for (auto i = 0; i < cnt; i++)
			{
				auto name = glsl.get_member_name(t.parent_type, i);
				auto offset = glsl.type_struct_member_offset(t, i);
				auto size = glsl.get_declared_struct_member_size(t, i);
				auto mid = t.member_types[i];
				auto mt = glsl.get_type(mid);
				auto count = mt.array.size() > 0 ? mt.array[0] : 1;
				auto array_stride = glsl.get_decoration(mid, spv::DecorationArrayStride);

				auto n = dst->new_node(name);
				n->new_attr("offset", std::to_string(offset));
				n->new_attr("size", std::to_string(size));
				n->new_attr("count", std::to_string(count));
				n->new_attr("array_stride", std::to_string(array_stride));
				serialize_members(glsl, mid, n);
			}
		}

		static void produce_shader_resource_file(const std::wstring &spv_file_in, const std::wstring &res_file_out)
		{
			auto spv_file = get_file_content(spv_file_in);

			std::vector<uint> spv_vec(spv_file.second / sizeof(uint));
			memcpy(spv_vec.data(), spv_file.first.get(), spv_file.second);

			spirv_cross::CompilerGLSL glsl(std::move(spv_vec));

			spirv_cross::ShaderResources resources = glsl.get_shader_resources();

			auto file = SerializableNode::create("res");

			for (auto &r : resources.uniform_buffers)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				auto size = glsl.get_declared_struct_size(type);

				auto n = file->new_node("uniform_buffer");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("size", std::to_string(size));
				n->new_attr("name", r.name);

				auto mn = n->new_node("members");
				serialize_members(glsl, r.type_id, mn);

			}
			for (auto &r : resources.storage_buffers)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				auto size = glsl.get_declared_struct_size(type);

				auto n = file->new_node("storage_buffer");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("size", std::to_string(size));
				n->new_attr("name", r.name);

				auto mn = n->new_node("members");
				serialize_members(glsl, r.type_id, mn);
			}
			for (auto &r : resources.sampled_images)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				int count = type.array.size() > 0 ? type.array[0] : 1;

				auto n = file->new_node("sampled_image");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("count", std::to_string(count));
				n->new_attr("name", r.name);
			}
			for (auto &r : resources.storage_images)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				int count = type.array.size() > 0 ? type.array[0] : 1;

				auto n = file->new_node("storage_image");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("count", std::to_string(count));
				n->new_attr("name", r.name);
			}
			for (auto &r : resources.push_constant_buffers)
			{
				auto offset = glsl.get_decoration(r.id, spv::DecorationOffset);

				auto type = glsl.get_type(r.type_id);
				auto size = glsl.get_declared_struct_size(type);

				auto n = file->new_node("push_constant");
				n->new_attr("offset", std::to_string(offset));
				n->new_attr("size", std::to_string(size));
				n->new_attr("name", r.name);

				auto mn = n->new_node("members");
				serialize_members(glsl, r.type_id, mn);
			}

			file->save_xml(res_file_out);
			SerializableNode::destroy(file);
		}
#endif

		ShaderPrivate::ShaderPrivate(Device *_d, const std::wstring &filename, const std::string &prefix)
		{
			filename_ = filename;
			prefix_ = prefix;
			auto ext = std::fs::path(filename).extension();
			if (ext == L".vert")
				type = ShaderVert;
			else if (ext == L".tesc")
				type = ShaderTesc;
			else if (ext == L".tese")
				type = ShaderTese;
			else if (ext == L".geom")
				type = ShaderGeom;
			else if (ext == L".frag")
				type = ShaderFrag;
			else if (ext == L".comp")
				type = ShaderComp;

			d = (DevicePrivate*)_d;

			std::fs::remove(L"temp.spv"); // glslc cannot write to an existed file (well we did delete it when we finish compiling, but there can be one somehow)

			auto glsl_path = std::fs::path(shader_path + L"src/" + filename);

			auto hash = H(prefix.c_str());
			std::wstring spv_filename(filename + L"." + std::to_wstring(hash) + L".spv");
			spv_filename = shader_path + L"bin/" + spv_filename;

			if (!std::fs::exists(spv_filename) || std::fs::last_write_time(spv_filename) <= std::fs::last_write_time(glsl_path))
			{
				auto vk_sdk_path = s2w(getenv("VK_SDK_PATH"));
				assert(vk_sdk_path != L"");

				std::string pfx;
				pfx += "#version 450 core\n";
				pfx += "#extension GL_ARB_shading_language_420pack : enable\n";  // Allows the setting of uniform buffer object and sampler binding points directly from GLSL
				if (type != ShaderComp)
					pfx += "#extension GL_ARB_separate_shader_objects : enable\n";
				pfx += "\n" + prefix;
				auto temp_filename = glsl_path.parent_path().wstring() + L"/temp." + glsl_path.filename().wstring();
				{
					std::ofstream ofile(temp_filename);
					auto file = get_file_content(glsl_path.wstring());
					ofile.write(pfx.c_str(), pfx.size());
					ofile.write(file.first.get(), file.second);
					ofile.close();
				}
				std::wstring command_line(L" " + temp_filename + L" -flimit-file " + conf_path + L" -o temp.spv");
				auto output = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
				std::fs::remove(temp_filename);
				if (!std::fs::exists("temp.spv"))
					printf("shader \"%s\" compile error:\n\n%s\n\n", glsl_path.string().c_str(), *output.p);
				else
				{
					auto spv_dir = std::fs::path(spv_filename).parent_path();
					if (!std::fs::exists(spv_dir))
						std::fs::create_directories(spv_dir);
					std::fs::copy_file("temp.spv", spv_filename, std::fs::copy_options::overwrite_existing);
					std::fs::remove("temp.spv");
				}
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
			shader_info.pCode = (uint32_t*)spv_file.first.get();
			chk_res(vkCreateShaderModule(d->v, &shader_info, nullptr, &v));

			auto res_filename = spv_filename + L".xml";
			if (!std::fs::exists(res_filename) || std::fs::last_write_time(res_filename) <= std::fs::last_write_time(spv_filename))
				produce_shader_resource_file(spv_filename.c_str(), res_filename.c_str());

#elif defined(FLAME_D3D12)

#endif
		}

		ShaderPrivate::~ShaderPrivate()
		{
#if defined(FLAME_VULKAN)
			if (v)
				vkDestroyShaderModule(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Shader *Shader::create(Device *d, const std::wstring &filename, const std::string &prefix)
		{
			return new ShaderPrivate(d, filename, prefix);
		}

		void Shader::destroy(Shader *s)
		{
			delete (ShaderPrivate*)s;
		}

		struct Shader$
		{
			AttributeP<void> device$i;
			AttributeV<std::wstring> filename$i;
			AttributeV<std::string> prefix$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Shader$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (device$i.frame > out$o.frame || filename$i.frame > out$o.frame || prefix$i.frame > out$o.frame)
				{
					if (out$o.v)
						Shader::destroy((Shader*)out$o.v);
					if (device$i.v && std::fs::exists(filename$i.v))
						out$o.v = Shader::create((Device*)device$i.v, filename$i.v, prefix$i.v);
					else
					{
						printf("cannot create shader\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(device$i.frame, filename$i.frame, prefix$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Shader$()
			{
				if (out$o.v)
					Shader::destroy((Shader*)out$o.v);
			}

		}bp_shader_unused;

		PipelinelayoutPrivate::PipelinelayoutPrivate(Device* _d, const std::vector<Descriptorsetlayout*>& _setlayouts, uint _pc_size)
		{
			d = (DevicePrivate*)_d;
			dsls.resize(_setlayouts.size());
			for (auto i = 0; i < dsls.size(); i++)
				dsls[i] = (DescriptorsetlayoutPrivate*)_setlayouts[i];
			pc_size = _pc_size;

#if defined(FLAME_VULKAN)
			std::vector<VkDescriptorSetLayout> vk_descriptorsetlayouts;
			vk_descriptorsetlayouts.resize(dsls.size());
			for (auto i = 0; i < vk_descriptorsetlayouts.size(); i++)
				vk_descriptorsetlayouts[i] = ((DescriptorsetlayoutPrivate*)dsls[i])->v;

			VkPushConstantRange vk_pushconstant;
			vk_pushconstant.offset = 0;
			vk_pushconstant.size = pc_size;
			vk_pushconstant.stageFlags = to_flags(ShaderAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptorsetlayouts.size();
			info.pSetLayouts = vk_descriptorsetlayouts.data();
			info.pushConstantRangeCount = vk_pushconstant.size > 0 ? 1 : 0;
			info.pPushConstantRanges = vk_pushconstant.size > 0 ? &vk_pushconstant : nullptr;

			chk_res(vkCreatePipelineLayout(d->v, &info, nullptr, &v));
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

		Descriptorsetlayout* Pipelinelayout::dsl(uint index) const
		{
			return ((PipelinelayoutPrivate*)this)->dsls[index];
		}

		Pipelinelayout* Pipelinelayout::create(Device* d, const std::vector<Descriptorsetlayout*>& setlayouts, uint pc_size)
		{
			return new PipelinelayoutPrivate(d, setlayouts, pc_size);
		}

		void Pipelinelayout::destroy(Pipelinelayout* l)
		{
			delete (PipelinelayoutPrivate*)l;
		}

		void PipelinePrivate::init()
		{
			vert_shader = nullptr;
			tesc_shader = nullptr;
			tese_shader = nullptr;
			geom_shader = nullptr;
			frag_shader = nullptr;
			comp_shader = nullptr;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, const GraphicsPipelineInfo& info)
		{
			init();

			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			for (auto& s : info.shaders)
				add_shader(s);

			auto vk_stage_infos = process_stages();

			auto vi_binding = 0;
			auto vi_location = 0;
			for (auto& b : info.vi_buffers)
			{
				VkVertexInputBindingDescription binding;
				binding.binding = vi_binding;
				binding.stride = 0;
				binding.inputRate = to_enum(b.rate);

				for (auto& a : b.attributes)
				{
					VkVertexInputAttributeDescription attribute;
					attribute.location = vi_location;
					attribute.binding = vi_binding;
					attribute.offset = binding.stride;
					attribute.format = to_enum(a);

					switch (a)
					{
					case Format_R8G8B8A8_UNORM:
						binding.stride += 4;
						break;
					case Format_R32_SFLOAT:
						binding.stride += 4;
						break;
					case Format_R32G32_SFLOAT:
						binding.stride += 8;
						break;
					case Format_R32G32B32_SFLOAT:
						binding.stride += 12;
						break;
					case Format_R32G32B32A32_SFLOAT:
						binding.stride += 16;
						break;
					default:
						assert(0);
					}

					vi_location++;
					vk_vi_attributes.push_back(attribute);
				}

				vi_binding++;
				vk_vi_bindings.push_back(binding);
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
			assembly_state.topology = to_enum(info.primitive_topology);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state;
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.pNext = nullptr;
			tess_state.flags = 0;
			tess_state.patchControlPoints = info.patch_control_points;

			VkViewport viewport;
			viewport.width = (float)info.viewport_size.x();
			viewport.height = (float)info.viewport_size.y();
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			viewport.x = 0;
			viewport.y = 0;

			VkRect2D scissor;
			scissor.extent.width = info.viewport_size.x();
			scissor.extent.height = info.viewport_size.y();
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
			raster_state.depthClampEnable = info.depth_clamp;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.polygonMode = to_enum(info.polygon_mode);
			raster_state.cullMode = to_enum(info.cull_mode);
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
			multisample_state.rasterizationSamples = to_enum(info.sample_count);
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.minSampleShading = 0.f;
			multisample_state.pSampleMask = nullptr;
			multisample_state.alphaToCoverageEnable = VK_FALSE;
			multisample_state.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = info.depth_test;
			depth_stencil_state.depthWriteEnable = info.depth_write;
			depth_stencil_state.depthCompareOp = to_enum(info.depth_compare_op);
			depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			depth_stencil_state.minDepthBounds = 0;
			depth_stencil_state.maxDepthBounds = 0;
			depth_stencil_state.stencilTestEnable = VK_FALSE;
			depth_stencil_state.front = {};
			depth_stencil_state.back = {};

			for (auto& a : info.blend_states)
			{
				VkPipelineColorBlendAttachmentState s;
				s.blendEnable = a.blend_enable;
				s.srcColorBlendFactor = to_enum(a.blend_src_color);
				s.dstColorBlendFactor = to_enum(a.blend_dst_color);
				s.colorBlendOp = VK_BLEND_OP_ADD;
				s.srcAlphaBlendFactor = to_enum(a.blend_src_alpha);
				s.dstAlphaBlendFactor = to_enum(a.blend_dst_alpha);
				s.alphaBlendOp = VK_BLEND_OP_ADD;
				s.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				vk_blend_attachment_states.push_back(s);
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

			for (auto& s : info.dynamic_states)
				vk_dynamic_states.push_back(to_enum(s));
			if (info.viewport_size.x() == 0 && info.viewport_size.y() == 0)
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
			pipeline_info.pTessellationState = info.patch_control_points > 0 ? &tess_state : nullptr;
			pipeline_info.pViewportState = &viewport_state;
			pipeline_info.pRasterizationState = &raster_state;
			pipeline_info.pMultisampleState = &multisample_state;
			pipeline_info.pDepthStencilState = &depth_stencil_state;
			pipeline_info.pColorBlendState = &blend_state;
			pipeline_info.pDynamicState = vk_dynamic_states.size() ? &dynamic_state : nullptr;
			pipeline_info.layout = layout->v;
			pipeline_info.renderPass = info.renderpass ? ((RenderpassPrivate*)info.renderpass)->v : nullptr;
			pipeline_info.subpass = info.subpass_index;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineGraphics;
		}

		PipelinePrivate::PipelinePrivate(Device* _d, const ShaderInfo& compute_shader)
		{
			init();

			d = (DevicePrivate*)_d;

			add_shader(compute_shader);

#if defined(FLAME_VULKAN)
			auto vk_stage_infos = process_stages();
			assert(vk_stage_infos.size() == 1);

			VkComputePipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipeline_info.flags = 0;
			pipeline_info.pNext = nullptr;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = layout->v;
			pipeline_info.stage = vk_stage_infos[0];

			chk_res(vkCreateComputePipelines(d->v, 0, 1, &pipeline_info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif

			type = PipelineCompute;
		}

		PipelinePrivate::~PipelinePrivate()
		{
			if (vert_shader)
				Shader::destroy(vert_shader);
			if (tesc_shader)
				Shader::destroy(tesc_shader);
			if (tese_shader)
				Shader::destroy(tese_shader);
			if (geom_shader)
				Shader::destroy(geom_shader);
			if (frag_shader)
				Shader::destroy(frag_shader);
			if (comp_shader)
				Shader::destroy(comp_shader);
			for (auto& l : dsls)
				Descriptorsetlayout::destroy(l);
			Pipelinelayout::destroy(layout);
#if defined(FLAME_VULKAN)
			vkDestroyPipeline(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void PipelinePrivate::add_shader(const ShaderInfo& info)
		{
			auto s = Shader::create(d, info.filename, info.prefix);
			switch (s->type)
			{
			case ShaderVert:
				vert_shader = (ShaderPrivate*)s;
				break;
			case ShaderTesc:
				tesc_shader = (ShaderPrivate*)s;
				break;
			case ShaderTese:
				tese_shader = (ShaderPrivate*)s;
				break;
			case ShaderGeom:
				geom_shader = (ShaderPrivate*)s;
				break;
			case ShaderFrag:
				frag_shader = (ShaderPrivate*)s;
				break;
			case ShaderComp:
				comp_shader = (ShaderPrivate*)s;
				break;
			default:
				assert(0);
			}
		}

#if defined(FLAME_VULKAN)
		static void process_stage(ShaderPrivate* s, std::vector<VkPipelineShaderStageCreateInfo>& stage_infos, std::vector<std::vector<Descriptorsetlayout::Binding>>& sets, uint& pc_size)
		{
			VkPipelineShaderStageCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.pSpecializationInfo = nullptr;
			info.pName = "main";
			info.stage = to_enum(s->type);
			info.module = s->v;
			stage_infos.push_back(info);

			for (auto& r : s->resources)
			{
				if (r->type == ShaderResourcePushConstant)
					pc_size = max(r->var.size, pc_size);
				else
				{
					if (r->set + 1 > sets.size())
						sets.resize(r->set + 1);
					Descriptorsetlayout::Binding b;
					b.binding = r->binding;
					b.type = r->type;
					b.count = r->var.count;
					sets[r->set].push_back(b);
				}
			}
		}

		std::vector<VkPipelineShaderStageCreateInfo> PipelinePrivate::process_stages()
		{
			std::vector<VkPipelineShaderStageCreateInfo> stage_infos;
			std::vector<std::vector<Descriptorsetlayout::Binding>> sets;
			auto pc_size = 0U;

			if (vert_shader)
				process_stage(vert_shader, stage_infos, sets, pc_size);
			if (tesc_shader)
				process_stage(tesc_shader, stage_infos, sets, pc_size);
			if (tese_shader)
				process_stage(tese_shader, stage_infos, sets, pc_size);
			if (geom_shader)
				process_stage(geom_shader, stage_infos, sets, pc_size);
			if (frag_shader)
				process_stage(frag_shader, stage_infos, sets, pc_size);
			if (comp_shader)
				process_stage(comp_shader, stage_infos, sets, pc_size);

			for (auto& g : sets)
			{
				auto l = Descriptorsetlayout::create(d, g);
				dsls.push_back(l);
			}
			layout = (PipelinelayoutPrivate*)Pipelinelayout::create(d, dsls, pc_size);

			return stage_infos;
		}
#elif defined(FLAME_D3D12)

#endif

		Pipelinelayout* Pipeline::layout() const
		{
			return ((PipelinePrivate*)this)->layout;
		}

		Pipeline* Pipeline::create(Device* d, const GraphicsPipelineInfo& info)
		{
			return new PipelinePrivate(d, info);
		}

		Pipeline* Pipeline::create(Device* d, const ShaderInfo& compute_shader)
		{
			return new PipelinePrivate(d, compute_shader);
		}

		void Pipeline::destroy(Pipeline* p)
		{
			delete (PipelinePrivate*)p;
		}

		struct PipelineGeneral2d$
		{
			AttributeP<void> device$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{

			}

			FLAME_GRAPHICS_EXPORTS ~PipelineGeneral2d$()
			{
				if (out$o.v)
					Pipeline::destroy((Pipeline*)out$o.v);
			}

		}bp_pipeline_unused;
	}
}
