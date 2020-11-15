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

		DescriptorPool* DescriptorPool::create(Device* device)
		{
			return new DescriptorPoolPrivate((DevicePrivate*)device);
		}

		DescriptorBindingPrivate::DescriptorBindingPrivate(uint index, const DescriptorBindingInfo& info) :
			binding(binding),
			type(info.type),
			count(info.count),
			name(info.name)
		{
		}

		std::vector<std::unique_ptr<DescriptorSetLayoutPrivate>> descriptor_set_layouts;

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

				bindings[i].reset(new DescriptorBindingPrivate(i, src));
			}

			VkDescriptorSetLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.bindingCount = vk_bindings.size();
			info.pBindings = vk_bindings.data();

			chk_res(vkCreateDescriptorSetLayout(device->vk_device, &info, nullptr, &vk_descriptor_set_layout));
		}

		DescriptorSetLayoutPrivate::DescriptorSetLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::vector<std::unique_ptr<ShaderType>>& _types, std::vector<std::unique_ptr<DescriptorBindingPrivate>>& _bindings) :
			device(device),
			filename(filename)
		{
			types.resize(_types.size());
			for (auto i = 0; i < types.size(); i++)
				types[i].reset(_types[i].release());
			bindings.resize(_bindings.size());
			for (auto i = 0; i < bindings.size(); i++)
				bindings[i].reset(_bindings[i].release());

			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			for (auto& src : bindings)
			{
				if (src)
				{
					VkDescriptorSetLayoutBinding dst;
					dst.binding = src->binding;
					dst.descriptorType = to_backend(src->type);
					dst.descriptorCount = max(src->count, 1U);
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

		int DescriptorSetLayoutPrivate::find_binding(const std::string& name)
		{
			for (auto i = 0; i < bindings.size(); i++)
			{
				if (bindings[i]->name == name)
					return i;
			}
			return -1;
		}

		DescriptorSetLayoutPrivate* DescriptorSetLayoutPrivate::get(DevicePrivate* device, const std::filesystem::path& filename)
		{
			for (auto& d : descriptor_set_layouts)
			{
				if (d->filename == filename)
					return d.get();
			}
			return DescriptorSetLayoutPrivate::create(device, filename);
		};

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
							dst->tag = ShaderTagInt;
							dst->name = "Int";
							break;
						case 2:
							dst->tag = ShaderTagVec2i;
							dst->name = "Vec2i";
							break;
						case 3:
							dst->tag = ShaderTagVec3i;
							dst->name = "Vec3i";
							break;
						case 4:
							dst->tag = ShaderTagVec4i;
							dst->name = "Vec4i";
							break;
						default:
							assert(0);
						}
						break;
					default:
						assert(0);
					}
					break;
				case spirv_cross::SPIRType::UInt:
					switch (src.columns)
					{
					case 1:
						switch (src.vecsize)
						{
						case 1:
							dst->tag = ShaderTagUint;
							dst->name = "Uint";
							break;
						case 2:
							dst->tag = ShaderTagVec2u;
							dst->name = "Vec2u";
							break;
						case 3:
							dst->tag = ShaderTagVec3u;
							dst->name = "Vec3u";
							break;
						case 4:
							dst->tag = ShaderTagVec4u;
							dst->name = "Vec4u";
							break;
						default:
							assert(0);
						}
						break;
					default:
						assert(0);
					}
					break;
				case spirv_cross::SPIRType::Float:
					switch (src.columns)
					{
					case 1:
						switch (src.vecsize)
						{
						case 1:
							dst->tag = ShaderTagFloat;
							dst->name = "float";
							break;
						case 2:
							dst->tag = ShaderTagVec2f;
							dst->name = "Vec2f";
							break;
						case 3:
							dst->tag = ShaderTagVec3f;
							dst->name = "Vec3f";
							break;
						case 4:
							dst->tag = ShaderTagVec4f;
							dst->name = "Vec4f";
							break;
						default:
							assert(0);
						}
						break;
					case 2:
						switch (src.vecsize)
						{
						case 2:
							dst->tag = ShaderTagMat2f;
							dst->name = "Mat2f";
							break;
						default:
							assert(0);
						}
						break;
					case 3:
						switch (src.vecsize)
						{
						case 3:
							dst->tag = ShaderTagMat3f;
							dst->name = "Mat3f";
							break;
						default:
							assert(0);
						}
						break;
					case 4:
						switch (src.vecsize)
						{
						case 4:
							dst->tag = ShaderTagMat4f;
							dst->name = "Mat4f";
							break;
						default:
							assert(0);
						}
						break;
					default:
						assert(0);
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

		DescriptorSetLayoutPrivate* DescriptorSetLayoutPrivate::create(DevicePrivate* device, const std::filesystem::path& _filename)
		{
			auto filename = _filename;
			filename.make_preferred();

			auto path = filename;
			if (!get_resource_path(path, L"assets\\shaders"))
			{
				wprintf(L"cannot find dsl: %s\n", filename.c_str());
				return nullptr;
			}

			auto res_path = path;
			res_path += L".res";

			std::vector<std::unique_ptr<ShaderType>> types;
			std::vector<std::unique_ptr<DescriptorBindingPrivate>> bindings;

			if (!std::filesystem::exists(res_path) || std::filesystem::last_write_time(res_path) < std::filesystem::last_write_time(path))
			{
				auto vk_sdk_path = getenv("VK_SDK_PATH");
				if (vk_sdk_path)
				{
					auto temp = basic_glsl_prefix();
					temp += "#define MAKE_DSL\n";
					std::ifstream dsl(path);
					while (!dsl.eof())
					{
						std::string line;
						std::getline(dsl, line);
						temp += line + "\n";
					}
					temp += "void main()\n{\n}\n";
					dsl.close();

					auto temp_fn = path;
					temp_fn.replace_filename(L"temp.frag");
					std::ofstream temp_file(temp_fn);
					temp_file << temp << std::endl;
					temp_file.close();

					if (std::filesystem::exists(L"a.spv"))
						std::filesystem::remove(L"a.spv");

					auto glslc_path = std::filesystem::path(vk_sdk_path);
					glslc_path /= L"Bin/glslc.exe";

					auto command_line = std::wstring(L" " + temp_fn.wstring());

					printf("compiling dsl: %s", path.string().c_str());

					std::string output;
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), &output);
					if (!std::filesystem::exists(L"a.spv"))
					{
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						assert(0);
						return nullptr;
					}

					printf(" done\n");

					auto spv = get_file_content(L"a.spv");
					auto glsl = spirv_cross::CompilerGLSL((uint*)spv.c_str(), spv.size() / sizeof(uint));
					auto resources = glsl.get_shader_resources();

					auto get_binding = [&](spirv_cross::Resource& r, DescriptorType type) {
						auto info = new ShaderType;
						types.emplace_back(info);
						info->id = r.base_type_id;
						get_shader_type(glsl, types, glsl.get_type(r.base_type_id), info);
						
						auto b = new DescriptorBindingPrivate;
						b->type = type;
						b->binding = glsl.get_decoration(r.id, spv::DecorationBinding);
						b->count = glsl.get_type(r.type_id).array[0];
						b->name = r.name;
						b->info = info;

						if (bindings.size() <= b->binding)
						{
							bindings.resize(b->binding + 1);
							bindings[b->binding].reset(b);
						}
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
					for (auto& b : bindings)
					{
						auto n_binding = n_bindings.append_child("binding");
						if (b)
						{
							n_binding.append_attribute("type").set_value((int)b->type);
							n_binding.append_attribute("binding").set_value(b->binding);
							n_binding.append_attribute("count").set_value(b->count);
							n_binding.append_attribute("name").set_value(b->name.c_str());
							n_binding.append_attribute("type_id").set_value(b->info->id);
						}
						else
							n_binding.append_attribute("binding").set_value("-1");
					}

					res.save_file(res_path.c_str());
				}
				else
				{
					printf("cannot find vk sdk\n");
					assert(0);
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
					assert(0);
					return nullptr;
				}

				load_res_types(root, types);

				for (auto n_binding : root.child("bindings"))
				{
					auto binding = n_binding.attribute("binding").as_int();
					if (binding != -1)
					{
						auto b = new DescriptorBindingPrivate;
						b->type = (DescriptorType)n_binding.attribute("type").as_int();
						b->binding = n_binding.attribute("binding").as_int();
						b->count = n_binding.attribute("count").as_uint();
						b->name = n_binding.attribute("name").value();
						b->info = find_type(types, n_binding.attribute("type_id").as_uint());
						bindings.emplace_back(b);
					}
					else
						bindings.emplace_back(nullptr);
				}
			}

			auto ret = new DescriptorSetLayoutPrivate(device, filename, types, bindings);
			descriptor_set_layouts.emplace_back(ret);
			return ret;
		}

		DescriptorSetLayout* DescriptorSetLayout::create(Device* device, uint binding_count, const DescriptorBindingInfo* bindings)
		{
			return new DescriptorSetLayoutPrivate((DevicePrivate*)device, { bindings, binding_count });
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
			write.descriptorType = to_backend(descriptor_layout->bindings[binding]->type);
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
			i.imageLayout = descriptor_layout->bindings[binding]->type == DescriptorSampledImage ? 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
			i.sampler = sp ? sp->vk_sampler : nullptr;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = vk_descriptor_set;
			write.dstBinding = binding;
			write.dstArrayElement = index;
			write.descriptorType = to_backend(descriptor_layout->bindings[binding]->type);
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

		std::vector<std::unique_ptr<PipelineLayoutPrivate>> pipeline_layouts;

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
				descriptor_set_layouts[i] = _descriptor_set_layouts[i];
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

		PipelineLayoutPrivate* PipelineLayoutPrivate::get(DevicePrivate* device, const std::filesystem::path& filename)
		{
			for (auto& p : pipeline_layouts)
			{
				if (p->filename == filename)
					return p.get();
			}
			return PipelineLayoutPrivate::create(device, filename);
		};

		PipelineLayoutPrivate* PipelineLayoutPrivate::create(DevicePrivate* device, const std::filesystem::path& _filename)
		{
			auto filename = _filename;
			filename.make_preferred();

			auto path = filename;
			if (!get_resource_path(path, L"assets\\shaders"))
			{
				wprintf(L"cannot find pll: %s\n", filename.c_str());
				return nullptr;
			}

			auto res_path = path;
			res_path += L".res";

			std::vector<DescriptorSetLayoutPrivate*> dsls;

			auto ppath = path.parent_path();
			auto dependencies = get_make_dependencies(path);
			for (auto& d : dependencies)
			{
				if (d.extension() == L".dsl")
				{
					auto fn = d.lexically_relative(ppath);
					dsls.push_back(DescriptorSetLayoutPrivate::get(device, fn));
				}
			}

			std::vector<std::unique_ptr<ShaderType>> types;
			ShaderType* push_constant = nullptr;

			if (!std::filesystem::exists(res_path) || std::filesystem::last_write_time(res_path) < std::filesystem::last_write_time(path))
			{
				auto vk_sdk_path = getenv("VK_SDK_PATH");
				if (vk_sdk_path)
				{
					auto temp = basic_glsl_prefix();
					temp += "#define MAKE_PLL\n";
					std::ifstream pll(path);
					while (!pll.eof())
					{
						std::string line;
						std::getline(pll, line);
						temp += line + "\n";
					}
					temp += "void main()\n{\n}\n";
					pll.close();

					auto temp_fn = path;
					temp_fn.replace_filename(L"temp.frag");
					std::ofstream temp_file(temp_fn);
					temp_file << temp << std::endl;
					temp_file.close();

					if (std::filesystem::exists(L"a.spv"))
						std::filesystem::remove(L"a.spv");

					auto glslc_path = std::filesystem::path(vk_sdk_path);
					glslc_path /= L"Bin/glslc.exe";

					auto command_line = std::wstring(L" " + temp_fn.wstring());

					printf("compiling pll: %s", path.string().c_str());

					std::string output;
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), &output);
					if (!std::filesystem::exists(L"a.spv"))
					{
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						assert(0);
						return nullptr;
					}

					printf(" done\n");

					auto spv = get_file_content(L"a.spv");
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
					assert(0);
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
					assert(0);
					return nullptr;
				}

				load_res_types(root, types);

				auto n_push_constant = root.child("push_constant");
				push_constant = find_type(types, n_push_constant.attribute("type_id").as_uint());
			}

			auto ret = new PipelineLayoutPrivate(device, filename, dsls, types, push_constant);
			pipeline_layouts.emplace_back(ret);
			return ret;
		}

		PipelineLayout* PipelineLayout::create(Device* device, uint descriptorlayout_count, DescriptorSetLayout* const* descriptorlayouts, uint push_constant_size)
		{
			return new PipelineLayoutPrivate((DevicePrivate*)device, { (DescriptorSetLayoutPrivate**)descriptorlayouts, descriptorlayout_count }, push_constant_size);
		}

		ShaderPrivate::ShaderPrivate(DevicePrivate* device, const std::filesystem::path& filename, const std::string& defines, const std::string& substitutes, const std::string& spv_content) :
			device(device),
			filename(filename),
			defines(defines),
			substitutes(substitutes)
		{
			type = shader_stage_from_ext(filename.extension());

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

		ShaderPrivate* ShaderPrivate::create(DevicePrivate* device, const std::filesystem::path& _filename, const std::string& defines, const std::string& substitutes)
		{
			auto filename = _filename;
			filename.make_preferred();

			auto path = filename;
			if (!get_resource_path(path, L"assets\\shaders"))
			{
				wprintf(L"cannot find shader: %s\n", filename.c_str());
				return nullptr;
			}

			auto hash = std::hash<std::wstring>()(filename) ^ std::hash<std::string>()(defines) ^ std::hash<std::string>()(substitutes);
			auto str_hash = std::to_wstring(hash);

			auto spv_path = path;
			spv_path += L"." + str_hash;

			if (should_remake(get_make_dependencies(path), spv_path))
			{
				auto vk_sdk_path = getenv("VK_SDK_PATH");
				if (vk_sdk_path)
				{
					std::vector<std::pair<std::string, std::string>> replace_pairs;
					{
						auto sp = SUS::split(substitutes);
						assert(sp.size() % 2 == 0);
						for (auto i = 0; i < sp.size(); i += 2)
							replace_pairs.emplace_back(sp[i], sp[i + 1]);
					}

					auto temp = basic_glsl_prefix();
					std::ifstream glsl(path);
					while (!glsl.eof())
					{
						std::string line;
						std::getline(glsl, line);
						for (auto& p : replace_pairs)
							SUS::replace_all(line, p.first, p.second);
						temp += line + "\n";
					}
					glsl.close();

					auto temp_fn = path;
					temp_fn.replace_filename(L"temp");
					temp_fn.replace_extension(filename.extension());
					std::ofstream temp_file(temp_fn);
					temp_file << temp << std::endl;
					temp_file.close();

					if (std::filesystem::exists(spv_path))
						std::filesystem::remove(spv_path);

					auto glslc_path = std::filesystem::path(vk_sdk_path);
					glslc_path /= L"Bin/glslc.exe";

					auto command_line = std::wstring(L" " + temp_fn.wstring() + L" -o" + spv_path.wstring());
					auto sp = SUS::split(defines);
					for (auto& d : sp)
						command_line += L" -D" + s2w(d);

					printf("compiling shader: %s (%s) (%s)", path.string().c_str(), defines.c_str(), substitutes.c_str());

					std::string output;
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), &output);
					if (!std::filesystem::exists(spv_path))
					{
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						assert(0);
						return nullptr;
					}

					printf(" done\n");

					//auto shader_info_path = std::filesystem::path(getenv("FLAME_PATH"));
					//shader_info_path /= L"bin/debug/shader_info.exe";
					//if (std::filesystem::exists(shader_info_path))
					//{
					//	exec(shader_info_path.c_str(), (wchar_t*)spv_path.c_str(), &output);
					//	if (output.find("ALIGNMENT DISMATCH") != std::string::npos)
					//	{
					//		printf("%s\n", output.c_str());
					//		assert(0);
					//		return nullptr;
					//	}
					//}
				}
				else
				{
					printf("cannot find vk sdk\n");
					assert(0);
					return nullptr;
				}
			}

			auto spv_file = get_file_content(spv_path);
			if (spv_file.empty())
			{
				assert(0);
				return nullptr;
			}

			return new ShaderPrivate(device, filename, defines, substitutes, spv_file);
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

			vk_stage_infos.resize(_shaders.size());
			for (auto i = 0; i < _shaders.size(); i++)
			{
				auto& src = _shaders[i];
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

		Pipeline* create(Device* device, uint shaders_count, Shader* const* shaders, PipelineLayout* pll, 
			Renderpass* rp, uint subpass_idx, VertexInfo* vi, RasterInfo* raster, DepthInfo* depth,
			uint blend_options_count, const BlendOption* blend_options,
			uint dynamic_states_count, const uint* dynamic_states)
		{
			return PipelinePrivate::create((DevicePrivate*)device, { (ShaderPrivate**)shaders, shaders_count }, (PipelineLayoutPrivate*)pll, 
				rp, subpass_idx, vi, raster, depth, 
				{ blend_options , blend_options_count }, { dynamic_states , dynamic_states_count });
		}

		Pipeline* create(Device* device, Shader* compute_shader, PipelineLayout* pll)
		{
			return PipelinePrivate::create((DevicePrivate*)device, (ShaderPrivate*)compute_shader, (PipelineLayoutPrivate*)pll);
		}
	}
}
