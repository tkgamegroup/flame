#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "../foundation/system.h"
#include "device_private.h"
#include "command_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

#include <spirv_glsl.hpp>

namespace flame
{
	namespace graphics
	{
		DescriptorPoolPrivate::DescriptorPoolPrivate(DevicePrivate* _device) :
			device(_device)
		{
			if (!device)
				device = default_device;

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
			descriptorPoolInfo.poolSizeCount = _countof(descriptorPoolSizes);
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
			descriptorPoolInfo.maxSets = 128;
			chk_res(vkCreateDescriptorPool(device->vk_device, &descriptorPoolInfo, nullptr, &vk_descriptor_pool));
		}

		DescriptorPoolPrivate::~DescriptorPoolPrivate()
		{
			vkDestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
		}

		DescriptorPool* DescriptorPool::get_default(Device* _device)
		{
			auto device = (DevicePrivate*)_device;
			if (!device)
				device = default_device;

			return device->dsp.get();
		}

		DescriptorPool* DescriptorPool::create(Device* device)
		{
			return new DescriptorPoolPrivate((DevicePrivate*)device);
		}

		TypeInfo* get_shader_type(const spirv_cross::CompilerGLSL& glsl, const spirv_cross::SPIRType& src, TypeInfoDataBase* db)
		{
			TypeInfo* ret = nullptr;

			if (src.basetype == spirv_cross::SPIRType::Struct)
			{
				auto name = glsl.get_name(src.self);
				auto size = glsl.get_declared_struct_size(src);
				{
					auto m = size % 16;
					if (m != 0)
						size += (16 - m);
				}

				auto ui = add_udt(name.c_str(), size, "", db);

				ret = TypeInfo::get(TypeData, name, db);

				for (auto i = 0; i < src.member_types.size(); i++)
				{
					uint32_t id = src.member_types[i];

					auto type = get_shader_type(glsl, glsl.get_type(id), db);
					auto name = glsl.get_member_name(src.self, i);
					auto offset = glsl.type_struct_member_offset(src, i);
					auto arr_size = glsl.get_type(id).array[0];
					auto arr_stride = glsl.get_decoration(id, spv::DecorationArrayStride);
					if (arr_stride == 0)
						arr_size = 1;
					ui->add_variable(type, name.c_str(), offset, arr_size, arr_stride, nullptr, "");
				}
			}
			else if (src.basetype == spirv_cross::SPIRType::Image || src.basetype == spirv_cross::SPIRType::SampledImage)
				ret = TypeInfo::get(TypePointer, "ShaderImage");
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
							ret = TypeInfo::get(TypeData, "int");
							break;
						case 2:
							ret = TypeInfo::get(TypeData, "glm::vec<2,int,0>");
							break;
						case 3:
							ret = TypeInfo::get(TypeData, "glm::vec<3,int,0>");
							break;
						case 4:
							ret = TypeInfo::get(TypeData, "glm::vec<4,int,0>");
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
							ret = TypeInfo::get(TypeData, "uint");
							break;
						case 2:
							ret = TypeInfo::get(TypeData, "glm::vec<2,uint,0>");
							break;
						case 3:
							ret = TypeInfo::get(TypeData, "glm::vec<3,uint,0>");
							break;
						case 4:
							ret = TypeInfo::get(TypeData, "glm::vec<4,uint,0>");
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
							ret = TypeInfo::get(TypeData, "float");
							break;
						case 2:
							ret = TypeInfo::get(TypeData, "glm::vec<2,float,0>");
							break;
						case 3:
							ret = TypeInfo::get(TypeData, "glm::vec<3,float,0>");
							break;
						case 4:
							ret = TypeInfo::get(TypeData, "glm::vec<4,float,0>");
							break;
						default:
							fassert(0);
						}
						break;
					case 2:
						switch (src.vecsize)
						{
						case 2:
							ret = TypeInfo::get(TypeData, "glm::mat<2,2,float,0>");
							break;
						default:
							fassert(0);
						}
						break;
					case 3:
						switch (src.vecsize)
						{
						case 3:
							ret = TypeInfo::get(TypeData, "glm::mat<3,3,float,0>");
							break;
						default:
							fassert(0);
						}
						break;
					case 4:
						switch (src.vecsize)
						{
						case 4:
							ret = TypeInfo::get(TypeData, "glm::mat<4,4,float,0>");
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

			return ret;
		}

		static void write_udts_to_header(std::string& header, TypeInfoDataBase* tidb)
		{
			std::vector<UdtInfo*> udts;
			{
				uint len;
				get_udts(nullptr, &len, tidb);
				udts.resize(len);
				get_udts(udts.data(), nullptr, tidb);
			}
			for (auto udt : udts)
			{
				header += std::string("\tstruct ") + udt->get_name() + "\n\t{\n";
				auto var_cnt = udt->get_variables_count();
				auto off = 0;
				auto dummy_id = 0;
				auto push_dummy = [&](int d) {
					switch (d)
					{
					case 4:
						header += "\t\tfloat dummy_" + std::to_string(dummy_id) + ";\n";
						break;
					case 8:
						header += "\t\tvec2 dummy_" + std::to_string(dummy_id) + ";\n";
						break;
					case 12:
						header += "\t\tvec3 dummy_" + std::to_string(dummy_id) + ";\n";
						break;
					default:
						fassert(0);
					}
				};
				for (auto i = 0; i < var_cnt; i++)
				{
					auto var = udt->get_variable(i);
					auto off2 = (int)var->get_offset();
					if (off != off2)
					{
						push_dummy(off2 - off);
						off = off2;
						dummy_id++;
					}
					auto type = var->get_type();
					std::string type_name;
					auto basic = type->get_basic();
					auto is_signed = type->get_signed();
					auto col_size = type->get_col_size();
					auto vec_size = type->get_vec_size();
					switch (col_size)
					{
					case 1:
						switch (vec_size)
						{
						case 1:
							switch (basic)
							{
							case IntegerType:
								if (is_signed)
									type_name = "int";
								else
									type_name = "uint";
								break;
							case FloatingType:
								type_name = "float";
								break;
							default:
								type_name = type->get_name();
								break;
							}
							break;
						case 2:
							switch (basic)
							{
							case IntegerType:
								if (is_signed)
									type_name = "ivec2";
								else
									type_name = "uvec2";
								break;
							case FloatingType:
								type_name = "vec2";
								break;
							}
							break;
						case 3:
							switch (basic)
							{
							case IntegerType:
								if (is_signed)
									type_name = "ivec3";
								else
									type_name = "uvec3";
								break;
							case FloatingType:
								type_name = "vec3";
								break;
							}
							break;
						case 4:
							switch (basic)
							{
							case IntegerType:
								if (is_signed)
									type_name = "ivec4";
								else
									type_name = "uvec4";
								break;
							case FloatingType:
								type_name = "vec4";
								break;
							}
							break;
						}
						break;
					case 2:
						switch (vec_size)
						{
						case 2:
							switch (basic)
							{
							case FloatingType:
								type_name = "mat2";
								break;
							}
							break;
						}
						break;
					case 3:
						switch (vec_size)
						{
						case 3:
							switch (basic)
							{
							case FloatingType:
								type_name = "mat3";
								break;
							}
							break;
						}
						break;
					case 4:
						switch (vec_size)
						{
						case 4:
							switch (basic)
							{
							case FloatingType:
								type_name = "mat4";
								break;
							}
							break;
						}
						break;
					}
					fassert(!type_name.empty());
					header += std::string("\t\t") + type_name + " " + var->get_name();
					auto size = type->get_size();
					auto array_size = var->get_array_size();
					if (array_size > 1)
					{
						fassert(size == var->get_array_stride());
						header += "[" + std::to_string(array_size) + "]";
					}
					header += ";\n";
					off += size * array_size;
				}
				auto size = (int)udt->get_size();
				if (off != size)
					push_dummy(size - off);
				header += "\t};\n\n";
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

		DescriptorSetLayoutPrivate::DescriptorSetLayoutPrivate(DevicePrivate* _device, std::span<const DescriptorBindingInfo> _bindings) :
			device(_device)
		{
			if (!device)
				device = default_device;

			bindings.resize(_bindings.size());

			std::vector<VkDescriptorSetLayoutBinding> vk_bindings(_bindings.size());
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

		DescriptorSetLayoutPrivate::DescriptorSetLayoutPrivate(DevicePrivate* _device, const std::filesystem::path& filename, 
			std::vector<DescriptorBinding>& _bindings, TypeInfoDataBase* db) :
			device(_device),
			filename(filename)
		{
			if (!device)
				device = default_device;

			bindings = _bindings;
			tidb.reset(db);

			std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
			for (auto i = 0; i < bindings.size(); i++)
			{
				auto& src = bindings[i];
				if (src.type != Descriptor_Max)
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

		int DescriptorSetLayoutPrivate::find_binding(const std::string& name) const
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
			if (!device)
				device = default_device;

			auto filename = _filename;
			if (!get_engine_path(filename, L"assets\\shaders"))
			{
				wprintf(L"cannot find dsl: %s\n", _filename.c_str());
				return nullptr;
			}
			filename.make_preferred();

			if (device)
			{
				for (auto& d : device->dsls)
				{
					if (d->filename.filename() == filename)
						return d.get();
				}
			}

			auto res_path = filename.parent_path() / L"build";
			if (!std::filesystem::exists(res_path))
				std::filesystem::create_directories(res_path);
			res_path /= filename.filename();
			res_path += L".res";

			auto ti_path = res_path;
			ti_path.replace_extension(L".typeinfo");

			std::vector<DescriptorBinding> bindings;
			TypeInfoDataBase* tidb = TypeInfoDataBase::create();

			auto ti_desctype = TypeInfo::get(TypeEnumSingle, "flame::graphics::DescriptorType");

			if (!std::filesystem::exists(res_path) || std::filesystem::last_write_time(res_path) < std::filesystem::last_write_time(filename) ||
				!std::filesystem::exists(ti_path) || std::filesystem::last_write_time(ti_path) < std::filesystem::last_write_time(filename))
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

					auto command_line = L" " + temp_fn.wstring();

					printf("compiling dsl: %s", filename.string().c_str());

					std::string output;
					output.reserve(4096);
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), output.data());
					if (!std::filesystem::exists(L"a.spv"))
					{
						temp = add_lineno_to_temp(temp);
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						return nullptr;
					}

					printf(" done\n");

					std::filesystem::remove(temp_fn);

					auto spv = get_file_content(L"a.spv");
					std::filesystem::remove(L"a.spv");
					auto glsl = spirv_cross::CompilerGLSL((uint*)spv.c_str(), spv.size() / sizeof(uint));
					auto resources = glsl.get_shader_resources();

					auto get_binding = [&](spirv_cross::Resource& r, DescriptorType type) {
						get_shader_type(glsl, glsl.get_type(r.base_type_id), tidb);

						auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);
						if (bindings.size() <= binding)
							bindings.resize(binding + 1);

						auto& b = bindings[binding];
						b.type = type;
						b.count = glsl.get_type(r.type_id).array[0];
						b.name = r.name;
						if (type == DescriptorUniformBuffer || type == DescriptorStorageBuffer)
							b.ti = find_udt(glsl.get_name(r.base_type_id).c_str(), tidb);
					};

					for (auto& r : resources.uniform_buffers)
						get_binding(r, DescriptorUniformBuffer);
					for (auto& r : resources.storage_buffers)
						get_binding(r, DescriptorStorageBuffer);
					for (auto& r : resources.sampled_images)
						get_binding(r, DescriptorSampledImage);
					for (auto& r : resources.storage_images)
						get_binding(r, DescriptorStorageImage);

					save_typeinfo(ti_path.c_str(), tidb);

					pugi::xml_document res;
					auto root = res.append_child("res");

					auto n_bindings = root.append_child("bindings");
					for (auto i = 0; i < bindings.size(); i++)
					{
						auto& b = bindings[i];
						if (b.type != Descriptor_Max)
						{
							auto n_binding = n_bindings.append_child("binding");
							n_binding.append_attribute("type").set_value(ti_desctype->serialize(&b.type).c_str());
							n_binding.append_attribute("binding").set_value(i);
							n_binding.append_attribute("count").set_value(b.count);
							n_binding.append_attribute("name").set_value(b.name.c_str());
							if (b.type == DescriptorUniformBuffer || b.type == DescriptorStorageBuffer)
								n_binding.append_attribute("type_name").set_value(b.ti->get_name());
						}
					}

					res.save_file(res_path.c_str());
				}
				else
				{
					printf("cannot find vk sdk\n");
					return nullptr;
				}
			}
			else
			{
				auto ti_path = res_path;
				ti_path.replace_extension(L".typeinfo");
				load_typeinfo(ti_path.c_str(), tidb);

				pugi::xml_document res;
				pugi::xml_node root;
				if (!res.load_file(res_path.c_str()) || (root = res.first_child()).name() != std::string("res"))
				{
					printf("res file wrong format\n");
					return nullptr;
				}

				for (auto n_binding : root.child("bindings"))
				{
					auto binding = n_binding.attribute("binding").as_int();
					if (binding != -1)
					{
						if (binding >= bindings.size())
							bindings.resize(binding + 1);
						auto& b = bindings[binding];
						ti_desctype->unserialize(&b.type, n_binding.attribute("type").value());
						b.count = n_binding.attribute("count").as_uint();
						b.name = n_binding.attribute("name").value();
						if (b.type == DescriptorUniformBuffer || b.type == DescriptorStorageBuffer)
							b.ti = find_udt(n_binding.attribute("type_name").value(), tidb);
					}
				}
			}

			auto header_path = filename;
			header_path += L".h";
			if (!std::filesystem::exists(header_path) || std::filesystem::last_write_time(header_path) < std::filesystem::last_write_time(ti_path))
			{
				std::string header;
				header += "namespace DSL_" + filename.filename().stem().string() + "\n{\n";
				write_udts_to_header(header, tidb);
				auto idx = 0;
				for (auto& b : bindings)
				{
					header += "\tinline uint " + b.name + "_binding = " + std::to_string(idx) + ";\n";
					header += "\tinline uint " + b.name + "_count = " + std::to_string(b.count) + ";\n";
					idx++;
				}
				header += "}\n";
				std::ofstream file(header_path);
				file << header;
				file.close();
			}

			if (device)
			{
				auto dsl = new DescriptorSetLayoutPrivate(device, filename, bindings, tidb);
				device->dsls.emplace_back(dsl);
				return dsl;
			}
			return nullptr;
		}

		DescriptorSetLayout* DescriptorSetLayout::create(Device* device, uint binding_count, const DescriptorBindingInfo* bindings)
		{
			return new DescriptorSetLayoutPrivate((DevicePrivate*)device, { bindings, binding_count });
		}

		DescriptorSetLayout* DescriptorSetLayout::get(Device* device, const wchar_t* filename)
		{
			return DescriptorSetLayoutPrivate::get((DevicePrivate*)device, filename);
		}

		DescriptorSetPrivate::DescriptorSetPrivate(DescriptorPoolPrivate* _pool, DescriptorSetLayoutPrivate* layout) :
			pool(_pool),
			layout(layout)
		{
			if (!pool)
				pool = (DescriptorPoolPrivate*)DescriptorPool::get_default();

			device = pool->device;

			reses.resize(layout->bindings.size());
			for (auto i = 0; i < reses.size(); i++)
				reses[i].resize(max(1U, layout->bindings[i].count));

			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = pool->vk_descriptor_pool;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &layout->vk_descriptor_set_layout;

			chk_res(vkAllocateDescriptorSets(device->vk_device, &info, &vk_descriptor_set));
		}

		DescriptorSetPrivate::~DescriptorSetPrivate()
		{
			chk_res(vkFreeDescriptorSets(device->vk_device, pool->vk_descriptor_pool, 1, &vk_descriptor_set));
		}

		void DescriptorSetPrivate::set_buffer(uint binding, uint index, BufferPtr buf, uint offset, uint range)
		{
			if (binding >= reses.size() || index >= reses[binding].size())
				return;
			auto& res = reses[binding][index].b;
			if (res.p == buf && res.offset == offset && res.range == range)
				return;

			res.p = buf;
			res.offset = offset;
			res.range = range;

			buf_updates.emplace_back(binding, index);
		}

		void DescriptorSetPrivate::set_image(uint binding, uint index, ImageViewPtr iv, SamplerPtr sp)
		{
			if (binding >= reses.size() || index >= reses[binding].size())
				return;
			auto& res = reses[binding][index].i;
			if (res.p == iv && res.sp == sp)
				return;

			res.p = iv;
			res.sp = sp;

			img_updates.emplace_back(binding, index);
		}

		void DescriptorSetPrivate::update()
		{
			if (buf_updates.empty() && img_updates.empty())
				return;

			Queue::get(device)->wait_idle();
			std::vector<VkDescriptorBufferInfo> vk_buf_infos;
			std::vector<VkDescriptorImageInfo> vk_img_infos;
			std::vector<VkWriteDescriptorSet> vk_writes;
			vk_buf_infos.resize(buf_updates.size());
			vk_img_infos.resize(img_updates.size());
			vk_writes.resize(buf_updates.size() + img_updates.size());
			auto idx = 0;
			for (auto i = 0; i < vk_buf_infos.size(); i++)
			{
				auto& u = buf_updates[i];
				auto& res = reses[u.first][u.second];

				auto& info = vk_buf_infos[i];
				info.buffer = res.b.p->vk_buffer;
				info.offset = res.b.offset;
				info.range = res.b.range == 0 ? res.b.p->size : res.b.range;

				auto& wrt = vk_writes[idx];
				wrt.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wrt.pNext = nullptr;
				wrt.dstSet = vk_descriptor_set;
				wrt.dstBinding = u.first;
				wrt.dstArrayElement = u.second;
				wrt.descriptorType = to_backend(layout->bindings[u.first].type);
				wrt.descriptorCount = 1;
				wrt.pBufferInfo = &info;
				wrt.pImageInfo = nullptr;
				wrt.pTexelBufferView = nullptr;

				idx++;
			}
			for (auto i = 0; i < vk_img_infos.size(); i++)
			{
				auto& u = img_updates[i];
				auto& res = reses[u.first][u.second];

				auto& info = vk_img_infos[i];
				info.imageView = res.i.p->vk_image_view;
				info.imageLayout = layout->bindings[u.first].type == DescriptorSampledImage ?
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
				info.sampler = res.i.sp ? res.i.sp->vk_sampler : nullptr;

				auto& wrt = vk_writes[idx];
				wrt.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wrt.pNext = nullptr;
				wrt.dstSet = vk_descriptor_set;
				wrt.dstBinding = u.first;
				wrt.dstArrayElement = u.second;
				wrt.descriptorType = to_backend(layout->bindings[u.first].type);
				wrt.descriptorCount = 1;
				wrt.pBufferInfo = nullptr;
				wrt.pImageInfo = &info;
				wrt.pTexelBufferView = nullptr;

				idx++;
			}

			buf_updates.clear();
			img_updates.clear();

			vkUpdateDescriptorSets(device->vk_device, vk_writes.size(), vk_writes.data(), 0, nullptr);
		}

		DescriptorSet* DescriptorSet::create(DescriptorPool* pool, DescriptorSetLayout* layout)
		{
			return new DescriptorSetPrivate((DescriptorPoolPrivate*)pool, (DescriptorSetLayoutPrivate*)layout);
		}

		PipelineLayoutPrivate::PipelineLayoutPrivate(DevicePrivate* _device, std::span<DescriptorSetLayoutPrivate*> _descriptor_set_layouts, uint push_constant_size) :
			device(_device),
			pc_sz(push_constant_size)
		{
			if (!device)
				device = default_device;

			descriptor_set_layouts.resize(_descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
				descriptor_set_layouts[i] = std::make_pair("", _descriptor_set_layouts[i]);

			std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts;
			vk_descriptor_set_layouts.resize(descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
				vk_descriptor_set_layouts[i] = descriptor_set_layouts[i].second->vk_descriptor_set_layout;

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

		PipelineLayoutPrivate::PipelineLayoutPrivate(DevicePrivate* _device, const std::filesystem::path& filename, 
			std::span<DescriptorSetLayoutPrivate*> _descriptor_set_layouts, TypeInfoDataBase* db, UdtInfo* _pcti) :
			device(_device),
			filename(filename)
		{
			if (!device)
				device = default_device;

			descriptor_set_layouts.resize(_descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
			{
				auto dsl = _descriptor_set_layouts[i];
				descriptor_set_layouts[i] = std::make_pair(dsl->filename.filename().stem().string(), dsl);
			}

			tidb.reset(db);
			pcti = _pcti;

			pc_sz = pcti ? pcti->get_size() : 0;

			std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts;
			vk_descriptor_set_layouts.resize(descriptor_set_layouts.size());
			for (auto i = 0; i < descriptor_set_layouts.size(); i++)
				vk_descriptor_set_layouts[i] = descriptor_set_layouts[i].second->vk_descriptor_set_layout;

			VkPushConstantRange vk_pushconstant_range;
			vk_pushconstant_range.offset = 0;
			vk_pushconstant_range.size = pc_sz;
			vk_pushconstant_range.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);

			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.setLayoutCount = vk_descriptor_set_layouts.size();
			info.pSetLayouts = vk_descriptor_set_layouts.data();
			info.pushConstantRangeCount = pc_sz > 0 ? 1 : 0;
			info.pPushConstantRanges = pc_sz > 0 ? &vk_pushconstant_range : nullptr;

			chk_res(vkCreatePipelineLayout(device->vk_device, &info, nullptr, &vk_pipeline_layout));
		}

		PipelineLayoutPrivate::~PipelineLayoutPrivate()
		{
			vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
		}

		PipelineLayoutPrivate* PipelineLayoutPrivate::get(DevicePrivate* device, const std::filesystem::path& _filename)
		{
			if (!device)
				device = default_device;

			auto filename = _filename;
			if (!get_engine_path(filename, L"assets\\shaders"))
			{
				wprintf(L"cannot find pll: %s\n", _filename.c_str());
				return nullptr;
			}
			filename.make_preferred();

			if (device)
			{
				for (auto& p : device->plls)
				{
					if (p->filename == filename)
						return p.get();
				}
			}

			auto res_path = filename.parent_path() / L"build";
			if (!std::filesystem::exists(res_path))
				std::filesystem::create_directories(res_path);
			res_path /= filename.filename();
			res_path += L".res";

			auto ti_path = res_path;
			ti_path.replace_extension(L".typeinfo");

			std::vector<DescriptorSetLayoutPrivate*> dsls;

			auto ppath = filename.parent_path();
			auto dependencies = get_make_dependencies(filename);
			for (auto& d : dependencies)
			{
				if (d.extension() == L".dsl")
					dsls.push_back(DescriptorSetLayoutPrivate::get(device, d));
				else
					d.clear();
			}

			auto tidb = TypeInfoDataBase::create();
			UdtInfo* pcti = nullptr;

			if (!std::filesystem::exists(res_path) || std::filesystem::last_write_time(res_path) < std::filesystem::last_write_time(filename) ||
				!std::filesystem::exists(ti_path) || std::filesystem::last_write_time(ti_path) < std::filesystem::last_write_time(filename))
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

					auto command_line = L" " + temp_fn.wstring();

					printf("compiling pll: %s", filename.string().c_str());

					std::string output;
					output.reserve(4096);
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), output.data());
					std::filesystem::remove(temp_fn);
					if (!std::filesystem::exists(L"a.spv"))
					{
						temp = add_lineno_to_temp(temp);
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						return nullptr;
					}

					printf(" done\n");

					auto spv = get_file_content(L"a.spv");
					std::filesystem::remove(L"a.spv");
					auto glsl = spirv_cross::CompilerGLSL((uint*)spv.c_str(), spv.size() / sizeof(uint));
					auto resources = glsl.get_shader_resources();

					for (auto& r : resources.push_constant_buffers)
					{
						get_shader_type(glsl, glsl.get_type(r.base_type_id), tidb);
						pcti = find_udt(glsl.get_name(r.base_type_id).c_str(), tidb);
						break;
					}
				}
				else
				{
					printf("cannot find vk sdk\n");
					return nullptr;
				}

				save_typeinfo(ti_path.c_str(), tidb);

				pugi::xml_document res;
				auto root = res.append_child("res");

				auto n_push_constant = root.append_child("push_constant");
				n_push_constant.append_attribute("type_name").set_value(pcti ? pcti->get_name() : "");

				res.save_file(res_path.c_str());
			}
			else
			{
				auto ti_path = res_path;
				ti_path.replace_extension(L".typeinfo");
				load_typeinfo(ti_path.c_str(), tidb);

				pugi::xml_document res;
				pugi::xml_node root;
				if (!res.load_file(res_path.c_str()) || (root = res.first_child()).name() != std::string("res"))
				{
					printf("res file wrong format\n");
					return nullptr;
				}

				auto n_push_constant = root.child("push_constant");
				pcti = find_udt(n_push_constant.attribute("type_name").value(), tidb);
			}

			auto header_path = filename;
			header_path += L".h";
			if (!std::filesystem::exists(header_path) || std::filesystem::last_write_time(header_path) < std::filesystem::last_write_time(ti_path))
			{
				std::string header;
				header += "namespace PLL_" + filename.filename().stem().string() + "\n{\n";
				header += "\tenum Binding\n\t{\n";
				for (auto& d : dependencies)
				{
					if (!d.empty())
						header += "\t\tBinding_" + d.filename().stem().string() + ",\n";
				}
				header += "\t\tBinding_Max\n";
				header += "\t};\n\n";
				write_udts_to_header(header, tidb);
				header += "}\n";
				std::ofstream file(header_path);
				file << header;
				file.close();
			}

			if (device)
			{
				auto pll = new PipelineLayoutPrivate(device, filename, dsls, tidb, pcti);
				device->plls.emplace_back(pll);
				return pll;
			}
			return nullptr;
		}

		PipelineLayout* PipelineLayout::create(Device* device, uint descriptorlayout_count, DescriptorSetLayout* const* descriptorlayouts, uint push_constant_size)
		{
			return new PipelineLayoutPrivate((DevicePrivate*)device, { (DescriptorSetLayoutPrivate**)descriptorlayouts, descriptorlayout_count }, push_constant_size);
		}

		PipelineLayout* PipelineLayout::get(Device* device, const wchar_t* filename)
		{
			return PipelineLayoutPrivate::get((DevicePrivate*)device, filename);
		}

		ShaderPrivate::ShaderPrivate(DevicePtr _device, const std::filesystem::path& filename, const std::vector<std::string>& defines,
			const std::vector<std::pair<std::string, std::string>>& substitutes, const std::string& spv_content) :
			device(_device),
			filename(filename),
			defines(defines),
			substitutes(substitutes)
		{
			if (!device)
				device = default_device;

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

		ShaderPrivate* ShaderPrivate::get(DevicePrivate* device, const std::filesystem::path& _filename, const std::vector<std::string>& _defines, 
			const std::vector<std::pair<std::string, std::string>>& _substitutes)
		{
			if (!device)
				device = default_device;

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

			if (device)
			{
				for (auto& s : device->sds)
				{
					if (s->filename == filename && s->defines == defines && s->substitutes == substitutes)
						return s.get();
				}
			}

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

			auto ppath = filename.parent_path();

			auto hash = 0U;
			for (auto& d : defines)
				hash = hash ^ std::hash<std::string>()(d);
			for (auto& s : substitutes)
				hash = hash ^ std::hash<std::string>()(s.first) ^ std::hash<std::string>()(s.second);
			auto str_hash = to_hex_wstring(hash);

			auto spv_path = ppath / L"build";
			if (!std::filesystem::exists(spv_path))
				std::filesystem::create_directories(spv_path);
			spv_path /= filename.filename();
			spv_path += L"." + str_hash + L".spv";

			auto dependencies = get_make_dependencies(filename);

			for (auto& s : substitutes)
			{
				if (s.first.ends_with("_FILE"))
				{
					auto fn = std::filesystem::path(s.second);
					if (!fn.is_absolute())
						fn = ppath / fn;
					s.second = get_file_content(fn);
					fassert(!s.second.empty());
					SUS::remove_ch(s.second, '\r');
					dependencies.push_back(fn);
				}
			}

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
							SUS::replace_all(line, s.first, s.second);
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

					auto command_line = L" " + temp_fn.wstring() + L" -o" + spv_path.wstring();
					for (auto& d : defines)
						command_line += L" -D" + s2w(d);

					printf("compiling shader: %s (%s) (%s)", filename.string().c_str(), defines_str.c_str(), substitutes_str.c_str());

					std::string output;
					output.reserve(4096);
					exec(glslc_path.c_str(), (wchar_t*)command_line.c_str(), output.data());
					if (!std::filesystem::exists(spv_path))
					{
						temp = add_lineno_to_temp(temp);
						printf("\n========\n%s\n========\n%s\n", temp.c_str(), output.c_str());
						return nullptr;
					}

					printf(" done\n");

					std::filesystem::remove(temp_fn);
				}
				else
				{
					printf("cannot find vk sdk\n");
					return nullptr;
				}
			}

			if (device)
			{
				auto s = new ShaderPrivate(device, filename, defines, substitutes, get_file_content(spv_path));
				device->sds.emplace_back(s);
				return s;
			}

			return nullptr;
		}

		ShaderPrivate* ShaderPrivate::get(DevicePrivate* device, const std::filesystem::path& filename, const std::string& defines, const std::string& substitutes)
		{
			return ShaderPrivate::get(device, filename, format_defines(defines), format_substitutes(substitutes));
		}

		Shader* Shader::get(Device* device, const wchar_t* filename, const char* defines, const char* substitutes)
		{
			return ShaderPrivate::get((DevicePrivate*)device, filename, defines, substitutes);
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* _device, const GraphicsPipelineInfo& info) :
			device(_device),
			layout((PipelineLayoutPrivate*)info.layout)
		{
			if (!device)
				device = default_device;

			type = PipelineGraphics;

			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
			std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			rp = (RenderpassPrivate*)info.renderpass;

			shaders.resize(info.shaders_count);
			vk_stage_infos.resize(shaders.size());
			for (auto i = 0; i < shaders.size(); i++)
			{
				auto shader = (ShaderPrivate*)info.shaders[i];

				auto& dst = vk_stage_infos[i];
				dst.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				dst.flags = 0;
				dst.pNext = nullptr;
				dst.pSpecializationInfo = nullptr;
				dst.pName = "main";
				dst.stage = to_backend(shader->type);
				dst.module = shader->vk_module;
				shaders[i] = shader;
			}

			if (info.vertex_buffers_count)
			{
				vk_vi_bindings.resize(info.vertex_buffers_count);
				for (auto i = 0; i < vk_vi_bindings.size(); i++)
				{
					auto& src_buf = info.vertex_buffers[i];
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
			assembly_state.topology = to_backend(info.primitive_topology);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state;
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.pNext = nullptr;
			tess_state.flags = 0;
			tess_state.patchControlPoints = info.patch_control_points;

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
			raster_state.depthClampEnable = VK_FALSE;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.polygonMode = to_backend(info.polygon_mode);
			raster_state.cullMode = to_backend(info.cull_mode);
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
			if (info.sample_count == SampleCount_1)
			{
				auto& res_atts = rp->subpasses[info.subpass_index].resolve_attachments;
				multisample_state.rasterizationSamples = to_backend(res_atts ? rp->attachments[res_atts[0]].sample_count : SampleCount_1);
			}
			else
				multisample_state.rasterizationSamples = to_backend(info.sample_count);
			multisample_state.sampleShadingEnable = VK_FALSE;
			multisample_state.minSampleShading = 0.f;
			multisample_state.pSampleMask = nullptr;
			multisample_state.alphaToCoverageEnable = info.alpha_to_coverage;
			multisample_state.alphaToOneEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = info.depth_test;
			depth_stencil_state.depthWriteEnable = info.depth_write;
			depth_stencil_state.depthCompareOp = to_backend(info.compare_op);
			depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			depth_stencil_state.minDepthBounds = 0;
			depth_stencil_state.maxDepthBounds = 0;
			depth_stencil_state.stencilTestEnable = VK_FALSE;
			depth_stencil_state.front = {};
			depth_stencil_state.back = {};

			vk_blend_attachment_states.resize(rp->subpasses[info.subpass_index].color_attachments_count);
			for (auto& a : vk_blend_attachment_states)
			{
				a.blendEnable = VK_FALSE;
				a.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				a.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				a.colorBlendOp = VK_BLEND_OP_ADD;
				a.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				a.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				a.alphaBlendOp = VK_BLEND_OP_ADD;
				a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}
			for (auto i = 0; i < info.blend_options_count; i++)
			{
				auto& src = info.blend_options[i];
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

			for (auto i = 0; i < info.dynamic_states_count; i++)
				vk_dynamic_states.push_back(to_backend((DynamicState)info.dynamic_states[i]));
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
			pipeline_info.layout = layout->vk_pipeline_layout;
			pipeline_info.renderPass = rp->vk_renderpass;
			pipeline_info.subpass = info.subpass_index;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;

			chk_res(vkCreateGraphicsPipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &vk_pipeline));
		}

		PipelinePrivate::PipelinePrivate(DevicePrivate* _device, const ComputePipelineInfo& info) :
			device(_device),
			layout((PipelineLayoutPrivate*)info.layout)
		{
			if (!device)
				device = default_device;

			type = PipelineCompute;

			auto shader = (ShaderPrivate*)info.shader;
			shaders.resize(1);
			shaders[0] = shader;

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
			pipeline_info.stage.module = shader->vk_module;

			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = layout->vk_pipeline_layout;

			chk_res(vkCreateComputePipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &vk_pipeline));
		}

		PipelinePrivate::~PipelinePrivate()
		{
			vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
		}

		PipelinePrivate* PipelinePrivate::create(DevicePrivate* device, const GraphicsPipelineInfo& info)
		{
			return new PipelinePrivate(device, info);
		}

		PipelinePrivate* PipelinePrivate::create(DevicePrivate* device, const ComputePipelineInfo& info)
		{
			return new PipelinePrivate(device, info);
		}

		PipelinePrivate* PipelinePrivate::get(DevicePrivate* device, const std::filesystem::path& _filename)
		{
			if (!device)
				device = default_device;

			auto filename = _filename;
			if (!get_engine_path(filename, L"assets\\shaders"))
			{
				wprintf(L"cannot find pipeline: %s\n", _filename.c_str());
				return nullptr;
			}
			filename.make_preferred();

			if (device)
			{
				for (auto& pl : device->pls)
				{
					if (pl->filename == filename)
						return pl.get();
				}
			}

			pugi::xml_document doc;
			pugi::xml_node doc_root;

			if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("pipeline"))
			{
				printf("pipeline wrong format: %s\n", _filename.string().c_str());
				return nullptr;
			}

			std::vector<ShaderPrivate*> shaders;
			for (auto n_shdr : doc_root.child("shaders"))
			{
				auto shader = ShaderPrivate::get(device, n_shdr.attribute("filename").value(), n_shdr.attribute("defines").value());
				fassert(shader);
				shaders.push_back(shader);
			}

			auto layout = PipelineLayoutPrivate::get(device, doc_root.child("layout").attribute("filename").value());
			fassert(layout);

			if (shaders.size() > 1)
			{
				GraphicsPipelineInfo info;

				info.shaders_count = shaders.size();
				info.shaders = (Shader**)shaders.data();

				info.layout = layout;

				auto n_rp = doc_root.child("renderpass");
				info.renderpass = RenderpassPrivate::get(device, n_rp.attribute("filename").value());
				fassert(info.renderpass);
				info.subpass_index = n_rp.attribute("index").as_uint();

				auto ti_format = TypeInfo::get(TypeEnumSingle, "flame::graphics::Format");
				std::vector<std::vector<VertexAttributeInfo>> v_vertex_attributes;
				std::vector<VertexBufferInfo> vertex_buffers;
				for (auto n_buf : doc_root.child("vertex_buffers"))
				{
					std::vector<VertexAttributeInfo> atts;
					for (auto n_att : n_buf)
					{
						VertexAttributeInfo att;
						att.location = n_att.attribute("location").as_uint();
						if (auto a = n_att.attribute("format"); a)
							ti_format->unserialize(&att.format, a.value());
						atts.push_back(att);
					}
					v_vertex_attributes.push_back(atts);

					VertexBufferInfo vb;
					vertex_buffers.push_back(vb);
				}
				for (auto i = 0; i < vertex_buffers.size(); i++)
				{
					auto& vb = vertex_buffers[i];
					auto& atts = v_vertex_attributes[i];
					vb.attributes_count = atts.size();
					vb.attributes = atts.data();
				}

				info.vertex_buffers_count = vertex_buffers.size();
				info.vertex_buffers = vertex_buffers.data();

				auto ti_prim = TypeInfo::get(TypeEnumSingle, "flame::graphics::PrimitiveTopology");
				auto ti_cullmode = TypeInfo::get(TypeEnumSingle, "flame::graphics::CullMode");
				auto ti_samplecount = TypeInfo::get(TypeEnumSingle, "flame::graphics::SampleCount");
				auto ti_compare = TypeInfo::get(TypeEnumSingle, "flame::graphics::CompareOp");
				if (auto n = doc_root.child("primitive_topology"); n)
					ti_prim->unserialize(&info.primitive_topology, n.attribute("v").value());
				if (auto n = doc_root.child("cull_mode"); n)
					ti_cullmode->unserialize(&info.cull_mode, n.attribute("v").value());
				if (auto n = doc_root.child("sample_count"); n)
					ti_samplecount->unserialize(&info.sample_count, n.attribute("v").value());
				if (auto n = doc_root.child("alpha_to_coverage"); n)
					info.alpha_to_coverage = n.attribute("v").as_bool();
				if (auto n = doc_root.child("depth_test"); n)
					info.depth_test = n.attribute("v").as_bool();
				if (auto n = doc_root.child("depth_write"); n)
					info.depth_write = n.attribute("v").as_bool();
				if (auto n = doc_root.child("compare_op"); n)
					ti_compare->unserialize(&info.compare_op, n.attribute("v").value());

				auto ti_blendfactor = TypeInfo::get(TypeEnumSingle, "flame::graphics::BlendFactor");
				std::vector<BlendOption> blend_options;
				for (auto n_bo : doc_root.child("blend_options"))
				{
					BlendOption bo;
					bo.enable = n_bo.attribute("enable").as_bool();
					if (auto a = n_bo.attribute("src_color"); a)
						ti_blendfactor->unserialize(&bo.src_color, a.value());
					if (auto a = n_bo.attribute("dst_color"); a)
						ti_blendfactor->unserialize(&bo.dst_color, a.value());
					if (auto a = n_bo.attribute("src_alpha"); a)
						ti_blendfactor->unserialize(&bo.src_alpha, a.value());
					if (auto a = n_bo.attribute("dst_alpha"); a)
						ti_blendfactor->unserialize(&bo.dst_alpha, a.value());
					blend_options.push_back(bo);
				}

				info.blend_options_count = blend_options.size();
				info.blend_options = blend_options.data();

				if (device)
				{
					auto pl = PipelinePrivate::create(device, info);
					pl->filename = filename;
					device->pls.emplace_back(pl);
					return pl;
				}
			}
			else
			{
				if (device)
				{
					ComputePipelineInfo info;

					info.shader = shaders[0];

					info.layout = layout;

					auto pl = PipelinePrivate::create(device, info);
					pl->filename = filename;
					device->pls.emplace_back(pl);
					return pl;
				}
			}

			return nullptr;
		}

		Pipeline* Pipeline::create(Device* device, const GraphicsPipelineInfo& info)
		{
			return PipelinePrivate::create((DevicePrivate*)device, info);
		}

		Pipeline* Pipeline::create(Device* device, const ComputePipelineInfo& info)
		{
			return PipelinePrivate::create((DevicePrivate*)device, info);
		}

		Pipeline* Pipeline::get(Device* device, const wchar_t* filename)
		{
			return PipelinePrivate::get((DevicePrivate*)device, filename);
		}
	}
}
