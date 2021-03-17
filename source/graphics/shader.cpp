#include <flame/foundation/typeinfo.h>
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#include <spirv_glsl.hpp>

namespace flame
{
	namespace graphics
	{
		void get_shader_type(const spirv_cross::CompilerGLSL& glsl, std::vector<std::unique_ptr<ShaderType>>& types, const spirv_cross::SPIRType& src, ShaderType* dst)
		{
			if (src.basetype == spirv_cross::SPIRType::Struct)
			{
				dst->tag = ShaderTagStruct;
				dst->name = glsl.get_name(src.self);
				{
					auto s = glsl.get_declared_struct_size(src);
					auto m = s % 16;
					if (m != 0)
						s += (16 - m);
					dst->size = s;
				}
				for (auto i = 0; i < src.member_types.size(); i++)
				{
					uint32_t id = src.member_types[i];

					ShaderVariable v;
					v.name = glsl.get_member_name(src.self, i);
					v.offset = glsl.type_struct_member_offset(src, i);
					v.size = glsl.get_declared_struct_member_size(src, i);
					v.array_count = glsl.get_type(id).array[0];
					v.array_stride = glsl.get_decoration(id, spv::DecorationArrayStride);
					ShaderType* t = nullptr;
					for (auto& _t : types)
					{
						if (_t->id == id)
						{
							t = _t.get();
							break;
						}
					}
					if (!t)
					{
						t = new ShaderType;
						types.emplace_back(t);
						t->id = id;
						get_shader_type(glsl, types, glsl.get_type(id), t);
					}
					v.info = t;

					dst->variables.push_back(v);
				}
				dst->make_map();
			}
			else if (src.basetype == spirv_cross::SPIRType::Image || src.basetype == spirv_cross::SPIRType::SampledImage)
				dst->tag = ShaderTagImage;
			else
			{
				switch (src.basetype)
				{
				case spirv_cross::SPIRType::Int:
					switch (src.columns)
					{
					case 1:
						switch (src.vecsize)
						{
						case 1:
							dst->tag = ShaderTagBase;
							dst->name = "int";
							dst->size = sizeof(int);
							break;
						case 2:
							dst->tag = ShaderTagBase;
							dst->name = "ivec2";
							dst->size = sizeof(ivec2);
							break;
						case 3:
							dst->tag = ShaderTagBase;
							dst->name = "ivec3";
							dst->size = sizeof(ivec3);
							break;
						case 4:
							dst->tag = ShaderTagBase;
							dst->name = "ivec4";
							dst->size = sizeof(ivec4);
							break;
						default:
							fassert(0);
						}
						break;
					default:
						fassert(0);
					}
					break;
				case spirv_cross::SPIRType::UInt:
					switch (src.columns)
					{
					case 1:
						switch (src.vecsize)
						{
						case 1:
							dst->tag = ShaderTagBase;
							dst->name = "uint";
							dst->size = sizeof(uint);
							break;
						case 2:
							dst->tag = ShaderTagBase;
							dst->name = "uvec2";
							dst->size = sizeof(uvec2);
							break;
						case 3:
							dst->tag = ShaderTagBase;
							dst->name = "uvec3";
							dst->size = sizeof(uvec3);
							break;
						case 4:
							dst->tag = ShaderTagBase;
							dst->name = "uvec4";
							dst->size = sizeof(uvec4);
							break;
						default:
							fassert(0);
						}
						break;
					default:
						fassert(0);
					}
					break;
				case spirv_cross::SPIRType::Float:
					switch (src.columns)
					{
					case 1:
						switch (src.vecsize)
						{
						case 1:
							dst->tag = ShaderTagBase;
							dst->name = "float";
							dst->size = sizeof(float);
							break;
						case 2:
							dst->tag = ShaderTagBase;
							dst->name = "vec2";
							dst->size = sizeof(vec2);
							break;
						case 3:
							dst->tag = ShaderTagBase;
							dst->name = "vec3";
							dst->size = sizeof(vec3);
							break;
						case 4:
							dst->tag = ShaderTagBase;
							dst->name = "vec4";
							dst->size = sizeof(vec4);
							break;
						default:
							fassert(0);
						}
						break;
					case 2:
						switch (src.vecsize)
						{
						case 2:
							dst->tag = ShaderTagBase;
							dst->name = "mat2";
							dst->size = sizeof(mat2);
							break;
						default:
							fassert(0);
						}
						break;
					case 3:
						switch (src.vecsize)
						{
						case 3:
							dst->tag = ShaderTagBase;
							dst->name = "mat3";
							dst->size = sizeof(mat3);
							break;
						default:
							fassert(0);
						}
						break;
					case 4:
						switch (src.vecsize)
						{
						case 4:
							dst->tag = ShaderTagBase;
							dst->name = "mat4";
							dst->size = sizeof(mat4);
							break;
						default:
							fassert(0);
						}
						break;
					default:
						fassert(0);
					}
					break;
				}
			}
		}

		static std::string basic_glsl_prefix()
		{
			std::string ret;
			ret += "#version 450 core\n";
			ret += "#extension GL_ARB_shading_language_420pack : enable\n";
			ret += "#extension GL_ARB_separate_shader_objects : enable\n\n";
			return ret;
		}

		static std::string add_lineno_to_temp(const std::string& temp)
		{
			auto lines = SUS::split(temp, '\n');
			auto ret = std::string();
			auto ndigits = (int)log10(lines.size()) + 1;
			for (auto i = 0; i < lines.size(); i++)
			{
				char buf[32];
				sprintf(buf, "%*d", ndigits, i + 1);
				ret += std::string(buf) + "    " + lines[i] + "\n";
			}
			return ret;
		}

		DescriptorPoolPrivate::DescriptorPoolPrivate(DevicePrivate* device) :
			device(device)
		{
			VkDescriptorPoolSize descriptorPoolSizes[] = {
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 512 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 8 },
			};

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.poolSizeCount = size(descriptorPoolSizes);
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 128;
			chk_res(vkCreateDescriptorPool(device->vk_device, &descriptorPoolInfo, nullptr, &vk_descriptor_pool));
		}

		DescriptorPoolPrivate::~DescriptorPoolPrivate()
		{
			vkDestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
		}

		DescriptorPool* DescriptorPool::get_default(Device* device)
		{
			return ((DevicePrivate*)device)->dsp.get();
		}

		DescriptorPool* DescriptorPool::create(Device* device)
		{
			return new DescriptorPoolPrivate((DevicePrivate*)device);
		}

		static void load_res_types(pugi::xml_node root, std::vector<std::unique_ptr<ShaderType>>& types)
		{
			for (auto n_type : root.child("types"))
			{
				auto t = new ShaderType;
				t->id = n_type.attribute("id").as_uint();
				t->tag = (ShaderTypeTag)n_type.attribute("tag").as_int();
				t->name = n_type.attribute("name").value();
				t->size = n_type.attribute("size").as_uint();
				for (auto n_variable : n_type.child("variables"))
				{
					ShaderVariable v;
					v.name = n_variable.attribute("name").value();
					v.offset = n_variable.attribute("offset").as_uint();
					v.size = n_variable.attribute("size").as_uint();
					v.array_count = n_variable.attribute("array_count").as_uint();
					v.array_stride = n_variable.attribute("array_stride").as_uint();
					v.info = (ShaderType*)n_variable.attribute("type_id").as_uint();
					t->variables.push_back(v);
				}
				t->make_map();
				types.emplace_back(t);
			}
			for (auto& t : types)
			{
				for (auto& v : t->variables)
					v.info = find_type(types, (uint)v.info);
			}
		}

		static void save_res_types(pugi::xml_node root, const std::vector<std::unique_ptr<ShaderType>>& types)
		{
			auto n_types = root.append_child("types");
			for (auto& t : types)
			{
				auto n_type = n_types.append_child("type");
				n_type.append_attribute("id").set_value(t->id);
				n_type.append_attribute("tag").set_value((int)t->tag);
				n_type.append_attribute("name").set_value(t->name.c_str());
				n_type.append_attribute("size").set_value(t->size);
				if (!t->variables.empty())
				{
					auto n_variables = n_type.append_child("variables");
					for (auto& v : t->variables)
					{
						auto n_variable = n_variables.append_child("variable");
						n_variable.append_attribute("name").set_value(v.name.c_str());
						n_variable.append_attribute("offset").set_value(v.offset);
						n_variable.append_attribute("size").set_value(v.size);
						n_variable.append_attribute("array_count").set_value(v.array_count);
						n_variable.append_attribute("array_stride").set_value(v.array_stride);
						n_variable.append_attribute("type_id").set_value(v.info->id);
					}
				}
			}
		}

		DescriptorSetLayoutPrivate::DescriptorSetLayoutPrivate(DevicePrivate* device, std::span<const DescriptorBindingInfo> _bindings) :
			device(device)
		{
			std::vector<VkDescriptorSetLayoutBinding> vk_bindings(_bindings.size());
			bindings.resize(_bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
			{
				auto& src = _bindings[i];
				auto& dst = vk_bindings[i];

				dst.binding = i;
				dst.descriptorType = to_backend(src.type);
				dst.descriptorCount = src.count;
				dst.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);
				dst.pImmutableSamplers = nullptr;

				auto& dst2 = bindings[i];
				dst2.type = src.type;
				dst2.count = src.count;
				dst2.name = src.name;
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			chk_res(vkCreateDescriptorSetLayout(device->vk_device, &info, nullptr, &vk_descriptor_set_layout));
		}

		DescriptorSetLayoutPrivate::DescriptorSetLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::vector<std::unique_ptr<ShaderType>>& _types, std::vector<DescriptorBinding>& _bindings) :
			device(device),
			filename(filename)
		{
			types.resize(_types.size());
			for (auto i = 0; i < types.size(); i++)
				types[i].reset(_types[i].release());
			bindings = _bindings;

			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			for (auto i = 0; i < bindings.size(); i++)
			{
				auto& src = bindings[i];
				if (src.type != DescriptorMax)
				{
					VkDescriptorSetLayoutBinding dst;
					dst.binding = i;
					dst.descriptorType = to_backend(src.type);
					dst.descriptorCount = max(src.count, 1U);
					dst.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);
					dst.pImmutableSamplers = nullptr;
					vk_bindings.push_back(dst);
				}
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			chk_res(vkCreateDescriptorSetLayout(device->vk_device, &info, nullptr, &vk_descriptor_set_layout));
		}

		DescriptorSetLayoutPrivate::~DescriptorSetLayoutPrivate()
		{
			vkDestroyDescriptorSetLayout(device->vk_device, vk_descriptor_set_layout, nullptr);
		}

		void DescriptorSetLayoutPrivate::get_binding(uint binding, DescriptorBindingInfo* ret) const
		{
			auto& b = bindings[binding];
			ret->type = b.type;
			ret->count = b.count;
			ret->name = b.name.c_str();
		}

		int DescriptorSetLayoutPrivate::find_binding(const std::string& name)
		{
			for (auto i = 0; i < bindings.size(); i++)
			{
				if (bindings[i].name == name)
					return i;
			}
			return -1;
		}

		DescriptorSetLayoutPrivate* DescriptorSetLayoutPrivate::get(DevicePrivate* device, const std::filesystem::path& _filename)
		{
			auto filename = _filename;
			if (!get_engine_path(filename, L"assets\\shaders"))
			{
				wprintf(L"cannot find dsl: %s\n", _filename.c_str());
				return nullptr;
			}
			filename.make_preferred();

			for (auto& d : device->dsls)
			{
				if (d->filename.filename() == filename)
					return d.get();
			}

			auto res_path = filename;
			res_path += L".res";

			std::vector<std::unique_ptr<ShaderType>> types;
			std::vector<DescriptorBinding> bindings;

			if (!std::filesystem::exists(res_path) || std::filesystem::last_write_time(res_path) < std::filesystem::last_write_time(filename))
			{
				auto vk_sdk_path = getenv("VK_SDK_PATH");
				if (vk_sdk_path)
				{
					auto temp = basic_glsl_prefix();
					temp += "#define MAKE_DSL\n";
					std::ifstream dsl(filename);
					while (!dsl.eof())
					{
						std::string line;
						std::getline(dsl, line);
						temp += line + "\n";
					}
					temp += "void main()\n{\n}\n";
					dsl.close();

					auto temp_fn = filename;
					temp_fn.replace_filename(L"temp.frag");
					std::ofstream temp_file(temp_fn);
					temp_file << temp << std::endl;
					temp_file.close();

					if (std::filesystem::exists(L"a.spv"))
						std::filesystem::remove(L"a.spv");

					auto glslc_path = std::filesystem::path(vk_sdk_path);
					glslc_path /= L"Bin/glslc.exe";

					auto command_line = std::wstring(L" -g " + temp_fn.wstring());

					printf("compiling dsl: %s", filename.string().c_str());

					std::string output;
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), &output, [](void* _str, uint size) {
						auto& str = *(std::string*)_str;
						str.resize(size);
						return str.data();
					});
					if (!std::filesystem::exists(L"a.spv"))
					{
						temp = add_lineno_to_temp(temp);
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						fassert(0);
						return nullptr;
					}

					printf(" done\n");

					std::filesystem::remove(temp_fn);

					auto spv = get_file_content(L"a.spv");
					std::filesystem::remove(L"a.spv");
					auto glsl = spirv_cross::CompilerGLSL((uint*)spv.c_str(), spv.size() / sizeof(uint));
					auto resources = glsl.get_shader_resources();

					auto get_binding = [&](spirv_cross::Resource& r, DescriptorType type) {
						auto info = new ShaderType;
						types.emplace_back(info);
						info->id = r.base_type_id;
						get_shader_type(glsl, types, glsl.get_type(r.base_type_id), info);

						auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);
						if (bindings.size() <= binding)
							bindings.resize(binding + 1);

						auto& b = bindings[binding];
						b.type = type;
						b.count = glsl.get_type(r.type_id).array[0];
						b.name = r.name;
						b.info = info;
					};

					for (auto& r : resources.uniform_buffers)
						get_binding(r, DescriptorUniformBuffer);
					for (auto& r : resources.storage_buffers)
						get_binding(r, DescriptorStorageBuffer);
					for (auto& r : resources.sampled_images)
						get_binding(r, DescriptorSampledImage);
					for (auto& r : resources.storage_images)
						get_binding(r, DescriptorStorageImage);

					pugi::xml_document res;
					auto root = res.append_child("res");

					save_res_types(root, types);

					auto n_bindings = root.append_child("bindings");
					for (auto i = 0; i < bindings.size(); i++)
					{
						auto& b = bindings[i];
						if (b.type != DescriptorMax)
						{
							auto n_binding = n_bindings.append_child("binding");
							n_binding.append_attribute("type").set_value(ti_es("flame::graphics::DescriptorType")->serialize(&b.type).c_str());
							n_binding.append_attribute("binding").set_value(i);
							n_binding.append_attribute("count").set_value(b.count);
							n_binding.append_attribute("name").set_value(b.name.c_str());
							n_binding.append_attribute("type_id").set_value(b.info->id);
						}
					}

					res.save_file(res_path.c_str());
				}
				else
				{
					printf("cannot find vk sdk\n");
					fassert(0);
					return nullptr;
				}
			}
			else
			{
				pugi::xml_document res;
				pugi::xml_node root;
				if (!res.load_file(res_path.c_str()) || (root = res.first_child()).name() != std::string("res"))
				{
					printf("res file does not exist\n");
					fassert(0);
					return nullptr;
				}

				load_res_types(root, types);

				for (auto n_binding : root.child("bindings"))
				{
					auto binding = n_binding.attribute("binding").as_int();
					if (binding != -1)
					{
						if (binding >= bindings.size())
							bindings.resize(binding + 1);
						auto& b = bindings[binding];
						ti_es("flame::graphics::DescriptorType")->unserialize(&b.type, n_binding.attribute("type").value());
						b.count = n_binding.attribute("count").as_uint();
						b.name = n_binding.attribute("name").value();
						b.info = find_type(types, n_binding.attribute("type_id").as_uint());
						bindings.emplace_back(b);
					}
				}
			}

			auto dsl = new DescriptorSetLayoutPrivate(device, filename, types, bindings);
			device->dsls.emplace_back(dsl);
			return dsl;
		}

		DescriptorSetLayout* DescriptorSetLayout::create(Device* device, uint binding_count, const DescriptorBindingInfo* bindings)
		{
			return new DescriptorSetLayoutPrivate((DevicePrivate*)device, { bindings, binding_count });
		}

		DescriptorSetLayout* DescriptorSetLayout::get(Device* device, const wchar_t* filename)
		{
			return DescriptorSetLayoutPrivate::get((DevicePrivate*)device, filename);
		}

		DescriptorSetPrivate::DescriptorSetPrivate(DescriptorPoolPrivate* p, DescriptorSetLayoutPrivate* l) :
			descriptor_pool(p),
			descriptor_layout(l)
		{
			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = p->vk_descriptor_pool;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &l->vk_descriptor_set_layout;

			chk_res(vkAllocateDescriptorSets(p->device->vk_device, &info, &vk_descriptor_set));
		}

		DescriptorSetPrivate::~DescriptorSetPrivate()
		{
			chk_res(vkFreeDescriptorSets(descriptor_pool->device->vk_device, descriptor_pool->vk_descriptor_pool, 1, &vk_descriptor_set));
		}

		void DescriptorSetPrivate::set_buffer(uint binding, uint index, BufferPrivate* b, uint offset, uint range)
		{
			VkDescriptorBufferInfo i;
			i.buffer = b->vk_buffer;
			i.offset = offset;
			i.range = range == 0 ? b->size : range;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = vk_descriptor_set;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = to_backend(descriptor_layout->bindings[binding].type);
			write.descriptorCount = 1;
			write.pBufferInfo = &i;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(descriptor_pool->device->vk_device, 1, &write, 0, nullptr);
		}

		void DescriptorSetPrivate::set_image(uint binding, uint index, ImageViewPrivate* iv, SamplerPrivate* sp)
		{
			VkDescriptorImageInfo i;
			i.imageView = iv->vk_image_view;
			i.imageLayout = descriptor_layout->bindings[binding].type == DescriptorSampledImage ? 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
			i.sampler = sp ? sp->vk_sampler : nullptr;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = vk_descriptor_set;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = to_backend(descriptor_layout->bindings[binding].type);
			write.descriptorCount = 1;
			write.pBufferInfo = nullptr;
			write.pImageInfo = &i;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(descriptor_pool->device->vk_device, 1, &write, 0, nullptr);
		}

		DescriptorSet* DescriptorSet::create(DescriptorPool* p, DescriptorSetLayout* l)
		{
			return new DescriptorSetPrivate((DescriptorPoolPrivate*)p, (DescriptorSetLayoutPrivate*)l);
		}

		PipelineLayoutPrivate::PipelineLayoutPrivate(DevicePrivate* device, std::span<DescriptorSetLayoutPrivate*> _descriptor_set_layouts, uint push_constant_size) :
			device(device)
		{
			descriptor_set_layouts.resize(_descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
				descriptor_set_layouts[i] = _descriptor_set_layouts[i];
			push_constant = new ShaderType;
			push_constant->size = push_constant_size;
			types.emplace_back(push_constant);

			std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts;
			vk_descriptor_set_layouts.resize(descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
				vk_descriptor_set_layouts[i] = descriptor_set_layouts[i]->vk_descriptor_set_layout;

			VkPushConstantRange vk_pushconstant_range;
			vk_pushconstant_range.offset = 0;
			vk_pushconstant_range.size = push_constant_size;
			vk_pushconstant_range.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptor_set_layouts.size();
			info.pSetLayouts = vk_descriptor_set_layouts.data();
			info.pushConstantRangeCount = push_constant_size > 0 ? 1 : 0;
			info.pPushConstantRanges = push_constant_size > 0 ? &vk_pushconstant_range : nullptr;

			chk_res(vkCreatePipelineLayout(device->vk_device, &info, nullptr, &vk_pipeline_layout));
		}

		PipelineLayoutPrivate::PipelineLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::span<DescriptorSetLayoutPrivate*> _descriptor_set_layouts, std::vector<std::unique_ptr<ShaderType>>& _types, ShaderType* _push_constant) :
			device(device),
			filename(filename)
		{
			descriptor_set_layouts.resize(_descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
			{
				auto dsl = _descriptor_set_layouts[i];
				descriptor_set_layouts[i] = dsl;
				auto fn = dsl->filename.filename().stem().string();
				descriptor_set_layouts_map[ch(fn.c_str())] = i;
			}


			types.resize(_types.size());
			for (auto i = 0; i < types.size(); i++)
				types[i].reset(_types[i].release());
			push_constant = _push_constant;
			if (!push_constant)
			{
				push_constant = new ShaderType;
				push_constant->size = 0;
				types.emplace_back(push_constant);
			}

			std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts;
			vk_descriptor_set_layouts.resize(descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
				vk_descriptor_set_layouts[i] = descriptor_set_layouts[i]->vk_descriptor_set_layout;

			VkPushConstantRange vk_pushconstant_range;
			vk_pushconstant_range.offset = 0;
			vk_pushconstant_range.size = push_constant->size;
			vk_pushconstant_range.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptor_set_layouts.size();
			info.pSetLayouts = vk_descriptor_set_layouts.data();
			info.pushConstantRangeCount = push_constant->size > 0 ? 1 : 0;
			info.pPushConstantRanges = push_constant->size > 0 ? &vk_pushconstant_range : nullptr;

			chk_res(vkCreatePipelineLayout(device->vk_device, &info, nullptr, &vk_pipeline_layout));
		}

		PipelineLayoutPrivate::~PipelineLayoutPrivate()
		{
			vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
		}

		PipelineLayoutPrivate* PipelineLayoutPrivate::get(DevicePrivate* device, const std::filesystem::path& _filename)
		{
			auto filename = _filename;
			if (!get_engine_path(filename, L"assets\\shaders"))
			{
				wprintf(L"cannot find pll: %s\n", _filename.c_str());
				return nullptr;
			}
			filename.make_preferred();

			for (auto& p : device->plls)
			{
				if (p->filename == filename)
					return p.get();
			}

			auto res_path = filename;
			res_path += L".res";

			std::vector<DescriptorSetLayoutPrivate*> dsls;

			auto ppath = filename.parent_path();
			auto dependencies = get_make_dependencies(filename);
			for (auto& d : dependencies)
			{
				if (d.extension() == L".dsl")
					dsls.push_back(DescriptorSetLayoutPrivate::get(device, d));
			}

			std::vector<std::unique_ptr<ShaderType>> types;
			ShaderType* push_constant = nullptr;

			if (!std::filesystem::exists(res_path) || std::filesystem::last_write_time(res_path) < std::filesystem::last_write_time(filename))
			{
				auto vk_sdk_path = getenv("VK_SDK_PATH");
				if (vk_sdk_path)
				{
					auto temp = basic_glsl_prefix();
					temp += "#define MAKE_PLL\n";
					std::ifstream pll(filename);
					while (!pll.eof())
					{
						std::string line;
						std::getline(pll, line);
						temp += line + "\n";
					}
					temp += "void main()\n{\n}\n";
					pll.close();

					auto temp_fn = filename;
					temp_fn.replace_filename(L"temp.frag");
					std::ofstream temp_file(temp_fn);
					temp_file << temp << std::endl;
					temp_file.close();

					if (std::filesystem::exists(L"a.spv"))
						std::filesystem::remove(L"a.spv");

					auto glslc_path = std::filesystem::path(vk_sdk_path);
					glslc_path /= L"Bin/glslc.exe";

					auto command_line = std::wstring(L" -g " + temp_fn.wstring());

					printf("compiling pll: %s", filename.string().c_str());

					std::string output;
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), &output, [](void* _str, uint size) {
						auto& str = *(std::string*)_str;
						str.resize(size);
						return str.data();
					});
					if (!std::filesystem::exists(L"a.spv"))
					{
						temp = add_lineno_to_temp(temp);
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						fassert(0);
						return nullptr;
					}

					printf(" done\n");

					std::filesystem::remove(temp_fn);

					auto spv = get_file_content(L"a.spv");
					std::filesystem::remove(L"a.spv");
					auto glsl = spirv_cross::CompilerGLSL((uint*)spv.c_str(), spv.size() / sizeof(uint));
					auto resources = glsl.get_shader_resources();

					for (auto& r : resources.push_constant_buffers)
					{
						auto info = new ShaderType;
						types.emplace_back(info);
						info->id = r.base_type_id;
						get_shader_type(glsl, types, glsl.get_type(r.base_type_id), info);

						push_constant = info;
						break;
					}
				}
				else
				{
					printf("cannot find vk sdk\n");
					fassert(0);
					return nullptr;
				}

				pugi::xml_document res;
				auto root = res.append_child("res");

				save_res_types(root, types);

				auto n_push_constant = root.append_child("push_constant");
				n_push_constant.append_attribute("type_id").set_value(push_constant ? std::to_string(push_constant->id).c_str() : "-1");

				res.save_file(res_path.c_str());
			}
			else
			{
				pugi::xml_document res;
				pugi::xml_node root;
				if (!res.load_file(res_path.c_str()) || (root = res.first_child()).name() != std::string("res"))
				{
					printf("res file does not exist\n");
					fassert(0);
					return nullptr;
				}

				load_res_types(root, types);

				auto n_push_constant = root.child("push_constant");
				push_constant = find_type(types, n_push_constant.attribute("type_id").as_uint());
			}

			auto pll = new PipelineLayoutPrivate(device, filename, dsls, types, push_constant);
			device->plls.emplace_back(pll);
			return pll;
		}

		PipelineLayout* PipelineLayout::create(Device* device, uint descriptorlayout_count, DescriptorSetLayout* const* descriptorlayouts, uint push_constant_size)
		{
			return new PipelineLayoutPrivate((DevicePrivate*)device, { (DescriptorSetLayoutPrivate**)descriptorlayouts, descriptorlayout_count }, push_constant_size);
		}

		PipelineLayout* PipelineLayout::get(Device* device, const wchar_t* filename)
		{
			return PipelineLayoutPrivate::get((DevicePrivate*)device, filename);
		}

		void ShaderPrivate::release()
		{
			for (auto it = device->sds.begin(); it != device->sds.end(); it++)
			{
				if (it->second.get() == this)
				{
					if (it->first == 1)
						device->sds.erase(it);
					else
						it->first--;
					return;
				}
			}
			delete this;
		}

		ShaderPrivate* ShaderPrivate::get(DevicePrivate* device, const std::filesystem::path& filename, const std::string& defines, const std::string& substitutes, const std::vector<std::filesystem::path>& extra_dependencies)
		{
			return ShaderPrivate::get(device, filename, format_defines(defines), format_substitutes(substitutes), extra_dependencies);
		}

		ShaderPrivate* ShaderPrivate::get(DevicePrivate* device, const std::filesystem::path& _filename, const std::vector<std::string>& _defines, const std::vector<std::pair<std::string, std::string>>& _substitutes, const std::vector<std::filesystem::path>& extra_dependencies)
		{
			auto filename = _filename;
			if (!get_engine_path(filename, L"assets\\shaders"))
			{
				wprintf(L"cannot find shader: %s\n", _filename.c_str());
				return nullptr;
			}
			filename.make_preferred();

			auto defines = _defines;
			std::sort(defines.begin(), defines.end());
			auto substitutes = _substitutes;
			std::sort(substitutes.begin(), substitutes.end(), [](const auto& a, const auto& b) {
				return a.first < b.first;
			});

			for (auto& s : device->sds)
			{
				if (s.second->filename == filename && s.second->defines == defines && s.second->substitutes == substitutes)
				{
					s.first++;
					return s.second.get();
				}
			}

			auto ppath = filename.parent_path();

			auto hash = std::hash<std::wstring>()(filename);
			for (auto& d : defines)
				hash = hash ^ std::hash<std::string>()(d);
			for (auto& s : substitutes)
				hash = hash ^ std::hash<std::string>()(s.first) ^ std::hash<std::string>()(s.second);
			auto str_hash = std::to_wstring(hash);

			auto spv_path = filename;
			spv_path += L"." + str_hash;

			auto dependencies = get_make_dependencies(filename);
			for (auto& e : extra_dependencies)
				dependencies.push_back(e.is_absolute() ? e : (ppath / e));
			if (should_remake(dependencies, spv_path))
			{
				auto vk_sdk_path = getenv("VK_SDK_PATH");
				if (vk_sdk_path)
				{
					auto temp = basic_glsl_prefix();
					std::ifstream glsl(filename);
					while (!glsl.eof())
					{
						std::string line;
						std::getline(glsl, line);
						for (auto& s : substitutes)
						{
							std::string content;
							if (s.first.ends_with("_FILE"))
							{
								auto fn = std::filesystem::path(s.second);
								if (!fn.is_absolute())
									fn = ppath / fn;
								content = get_file_content(fn);
								fassert(!content.empty());
								SUS::remove_ch(content, '\r');
							}
							else
								content = s.second;
							SUS::replace_all(line, s.first, content);
						}
						temp += line + "\n";
					}
					glsl.close();

					auto temp_fn = filename;
					temp_fn.replace_filename(L"temp");
					temp_fn.replace_extension(filename.extension());
					std::ofstream temp_file(temp_fn);
					temp_file << temp << std::endl;
					temp_file.close();

					if (std::filesystem::exists(spv_path))
						std::filesystem::remove(spv_path);

					auto glslc_path = std::filesystem::path(vk_sdk_path);
					glslc_path /= L"Bin/glslc.exe";

					auto command_line = std::wstring(L" -g " + temp_fn.wstring() + L" -o" + spv_path.wstring());
					for (auto& d : defines)
						command_line += L" -D" + s2w(d);

					{
						std::string defines_str;
						std::string substitutes_str;
						for (auto i = 0; i < defines.size(); i++)
						{
							defines_str += defines[i];
							if (i < defines.size() - 1)
								defines_str += " ";
						}
						for (auto i = 0; i < substitutes.size(); i++)
						{
							substitutes_str += substitutes[i].first + " " + substitutes[i].second;
							if (i < substitutes.size() - 1)
								substitutes_str += " ";
						}
						printf("compiling shader: %s (%s) (%s)", filename.string().c_str(), defines_str.c_str(), substitutes_str.c_str());
					}

					std::string output;
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), &output, [](void* _str, uint size) {
						auto& str = *(std::string*)_str;
						str.resize(size);
						return str.data();
					});
					if (!std::filesystem::exists(spv_path))
					{
						temp = add_lineno_to_temp(temp);
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						fassert(0);
						return nullptr;
					}

					printf(" done\n");

					std::filesystem::remove(temp_fn);
				}
				else
				{
					printf("cannot find vk sdk\n");
					fassert(0);
					return nullptr;
				}
			}

			auto spv_file = get_file_content(spv_path);
			if (spv_file.empty())
			{
				fassert(0);
				return nullptr;
			}

			auto s = new ShaderPrivate(device, filename, defines, substitutes, spv_file);
			device->sds.emplace_back(1, s);
			return s;
		}

		ShaderPrivate::ShaderPrivate(DevicePrivate* device, const std::filesystem::path& filename, const std::vector<std::string>& defines, const std::vector<std::pair<std::string, std::string>>& substitutes, const std::string& spv_content) :
			device(device),
			filename(filename),
			defines(defines),
			substitutes(substitutes)
		{
			auto ext = filename.extension();
			if (ext == L".vert")
				type = ShaderStageVert;
			else if (ext == L".tesc")
				type = ShaderStageTesc;
			else if (ext == L".tese")
				type = ShaderStageTese;
			else if (ext == L".geom")
				type = ShaderStageGeom;
			else if (ext == L".frag")
				type = ShaderStageFrag;
			else if (ext == L".comp")
				type = ShaderStageComp;

			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = spv_content.size();
			shader_info.pCode = (uint*)spv_content.data();
			chk_res(vkCreateShaderModule(device->vk_device, &shader_info, nullptr, &vk_module));
		}

		ShaderPrivate::~ShaderPrivate()
		{
			if (vk_module)
				vkDestroyShaderModule(device->vk_device, vk_module, nullptr);
		}

		Shader* Shader::get(Device* device, const wchar_t* filename, const char* defines, const char* substitutes)
		{
			return ShaderPrivate::get((DevicePrivate*)device, filename, defines, substitutes);
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* device, std::span<ShaderPrivate*> _shaders, PipelineLayoutPrivate* pll,
			RenderpassPrivate* rp, uint subpass_idx, VertexInfo* vi, RasterInfo* raster, DepthInfo* depth, 
			std::span<const BlendOption> blend_options, std::span<const uint> dynamic_states) :
			device(device),
			pipeline_layout(pll)
		{
			type = PipelineGraphics;

			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			shaders.resize(_shaders.size());
			vk_stage_infos.resize(_shaders.size());
			for (auto i = 0; i < _shaders.size(); i++)
			{
				auto src = _shaders[i];
				shaders[i] = src;

				auto& dst = vk_stage_infos[i];
				dst.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				dst.flags = 0;
				dst.pNext = nullptr;
				dst.pSpecializationInfo = nullptr;
				dst.pName = "main";
				dst.stage = to_backend(src->type);
				dst.module = src->vk_module;
			}

			if (vi)
			{
				vk_vi_bindings.resize(vi->buffers_count);
				for (auto i = 0; i < vk_vi_bindings.size(); i++)
				{
					auto& src_buf = vi->buffers[i];
					auto& dst_buf = vk_vi_bindings[i];
					dst_buf.binding = i;
					auto offset = 0;
					for (auto j = 0; j < src_buf.attributes_count; j++)
					{
						auto& src_att = src_buf.attributes[j];
						VkVertexInputAttributeDescription dst_att;
						dst_att.location = src_att.location;
						dst_att.binding = i;
						if (src_att.offset != -1)
							offset = src_att.offset;
						dst_att.offset = offset;
						offset += format_size(src_att.format);
						dst_att.format = to_backend(src_att.format);
						vk_vi_attributes.push_back(dst_att);
					}
					dst_buf.inputRate = to_backend(src_buf.rate);
					dst_buf.stride = src_buf.stride ? src_buf.stride : offset;
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
			assembly_state.topology = to_backend(vi ? vi->primitive_topology : PrimitiveTopologyTriangleList);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state;
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.pNext = nullptr;
			tess_state.flags = 0;
			tess_state.patchControlPoints = vi ? vi->patch_control_points : 0;

			VkViewport viewport;
			viewport.width = 1.f;
			viewport.height = 1.f;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			viewport.x = 0;
			viewport.y = 0;

			VkRect2D scissor;
			scissor.extent.width = 1;
			scissor.extent.height = 1;
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
			raster_state.depthClampEnable = raster ? raster->depth_clamp : false;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.polygonMode = to_backend(raster ? raster->polygon_mode : PolygonModeFill);
			raster_state.cullMode = to_backend(raster ? raster->cull_mode : CullModeNone);
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
			{
				auto& res_atts = rp->subpasses[subpass_idx]->resolve_attachments;
				multisample_state.rasterizationSamples = to_backend(res_atts.empty() ? SampleCount_1 : res_atts[0]->sample_count);
			}
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.minSampleShading = 0.f;
			multisample_state.pSampleMask = nullptr;
			multisample_state.alphaToCoverageEnable = VK_FALSE;
			multisample_state.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = depth ? depth->test : false;
			depth_stencil_state.depthWriteEnable = depth ? depth->write : false;
			depth_stencil_state.depthCompareOp = to_backend(depth ? depth->compare_op : CompareOpLess);
			depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			depth_stencil_state.minDepthBounds = 0;
			depth_stencil_state.maxDepthBounds = 0;
			depth_stencil_state.stencilTestEnable = VK_FALSE;
			depth_stencil_state.front = {};
			depth_stencil_state.back = {};

			vk_blend_attachment_states.resize(rp->subpasses[subpass_idx]->color_attachments.size());
			for (auto& a : vk_blend_attachment_states)
			{
				a = {};
				a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}
			for (auto i = 0; i < blend_options.size(); i++)
			{
				auto& src = blend_options[i];
				auto& dst = vk_blend_attachment_states[i];
				dst.blendEnable = src.enable;
				dst.srcColorBlendFactor = to_backend(src.src_color);
				dst.dstColorBlendFactor = to_backend(src.dst_color);
				dst.colorBlendOp = VK_BLEND_OP_ADD;
				dst.srcAlphaBlendFactor = to_backend(src.src_alpha);
				dst.dstAlphaBlendFactor = to_backend(src.dst_alpha);
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

			for (auto i = 0; i < dynamic_states.size(); i++)
				vk_dynamic_states.push_back(to_backend((DynamicState)dynamic_states[i]));
			if (std::find(vk_dynamic_states.begin(), vk_dynamic_states.end(), VK_DYNAMIC_STATE_VIEWPORT) == vk_dynamic_states.end())
				vk_dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			if (std::find(vk_dynamic_states.begin(), vk_dynamic_states.end(), VK_DYNAMIC_STATE_SCISSOR) == vk_dynamic_states.end())
				vk_dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);

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
			pipeline_info.layout = pll->vk_pipeline_layout;
			pipeline_info.renderPass = rp ? rp->vk_renderpass : nullptr;
			pipeline_info.subpass = subpass_idx;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &vk_pipeline));
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* device, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll) :
			device(device),
			pipeline_layout(pll)
		{
			type = PipelineCompute;

			shaders.resize(1);
			shaders[0] = compute_shader;

			VkComputePipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipeline_info.pNext = nullptr;
			pipeline_info.flags = 0;

			pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipeline_info.stage.flags = 0;
			pipeline_info.stage.pNext = nullptr;
			pipeline_info.stage.pSpecializationInfo = nullptr;
			pipeline_info.stage.pName = "main";
			pipeline_info.stage.stage = to_backend(ShaderStageComp);
			pipeline_info.stage.module = compute_shader->vk_module;

			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = pll->vk_pipeline_layout;

			chk_res(vkCreateComputePipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &vk_pipeline));
		}

		PipelinePrivate::~PipelinePrivate()
		{
			vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
		}

		PipelinePrivate* PipelinePrivate::create(DevicePrivate* device, std::span<ShaderPrivate*> shaders, PipelineLayoutPrivate* pll, 
			Renderpass* rp, uint subpass_idx, VertexInfo* vi, RasterInfo* raster, DepthInfo* depth, 
			std::span<const BlendOption> blend_options, std::span<const uint> dynamic_states)
		{
			auto has_vert_stage = false;
			auto tess_stage_count = 0;
			for (auto& s : shaders)
			{
				for (auto& ss : shaders)
				{
					if (ss != s && (ss->filename == s->filename || ss->type == s->type))
						return nullptr;
				}
				if (s->type == ShaderStageComp)
					return nullptr;
				if (s->type == ShaderStageVert)
					has_vert_stage = true;
				if (s->type == ShaderStageTesc || s->type == ShaderStageTese)
					tess_stage_count++;
			}
			if (!has_vert_stage || (tess_stage_count != 0 && tess_stage_count != 2))
				return nullptr;

			return new PipelinePrivate(device, shaders, pll, (RenderpassPrivate*)rp, subpass_idx, vi, raster, depth, blend_options, dynamic_states);
		}

		PipelinePrivate* PipelinePrivate::create(DevicePrivate* device, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll)
		{
			if (compute_shader->type != ShaderStageComp)
				return nullptr;

			return new PipelinePrivate(device, compute_shader, pll);
		}

		Pipeline* Pipeline::create(Device* device, uint shaders_count, Shader* const* shaders, PipelineLayout* pll,
			Renderpass* rp, uint subpass_idx, VertexInfo* vi, RasterInfo* raster, DepthInfo* depth,
			uint blend_options_count, const BlendOption* blend_options,
			uint dynamic_states_count, const uint* dynamic_states)
		{
			return PipelinePrivate::create((DevicePrivate*)device, { (ShaderPrivate**)shaders, shaders_count }, (PipelineLayoutPrivate*)pll, 
				rp, subpass_idx, vi, raster, depth, 
				{ blend_options , blend_options_count }, { dynamic_states , dynamic_states_count });
		}

		Pipeline* Pipeline::create(Device* device, Shader* compute_shader, PipelineLayout* pll)
		{
			return PipelinePrivate::create((DevicePrivate*)device, (ShaderPrivate*)compute_shader, (PipelineLayoutPrivate*)pll);
		}
	}
}
