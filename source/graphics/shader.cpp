#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "../foundation/typeinfo_serialize.h"
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
		std::wstring get_stage_str(ShaderStageFlags stage)
		{
			switch (stage)
			{
			case ShaderStageVert:
				return L"vert";
			case ShaderStageTesc:
				return L"tesc";
			case ShaderStageTese:
				return L"tese";
			case ShaderStageGeom:
				return L"geom";
			case ShaderStageFrag:
			case ShaderStageDsl:
			case ShaderStagePll:
				return L"frag";
			}
			return L"";
		};

		ShaderStageFlags stage_from_ext(const std::filesystem::path& fn)
		{
			auto ext = fn.extension();
			if (ext == L".vert")
				return ShaderStageVert;
			else if (ext == L".tesc")
				return ShaderStageTesc;
			else if (ext == L".tese")
				return ShaderStageTese;
			else if (ext == L".geom")
				return ShaderStageGeom;
			else if (ext == L".frag")
				return ShaderStageFrag;
			else if (ext == L".comp")
				return ShaderStageComp;
			return ShaderStageNone;
		}

		TypeInfo* get_shader_type(const spirv_cross::CompilerGLSL& compiler, spirv_cross::TypeID tid, TypeInfoDataBase& db)
		{
			TypeInfo* ret = nullptr;

			auto src = compiler.get_type(tid);
			if (src.basetype == spirv_cross::SPIRType::Struct)
			{
				auto name = compiler.get_name(src.self);
				auto size = compiler.get_declared_struct_size(src);
				{
					auto m = size % 16;
					if (m != 0)
						size += (16 - m);
				}

				UdtInfo ui;
				ui.name = name;
				ui.size = size;

				ret = TypeInfo::get(TagD, name, db);

				for (auto i = 0; i < src.member_types.size(); i++)
				{
					auto id = src.member_types[i];

					auto type = get_shader_type(compiler, id, db);
					auto name = compiler.get_member_name(src.self, i);
					auto offset = compiler.type_struct_member_offset(src, i);
					auto arr_size = compiler.get_type(id).array[0];
					auto arr_stride = compiler.get_decoration(id, spv::DecorationArrayStride);
					if (arr_stride == 0)
						arr_size = 1;
					auto& vi = ui.variables.emplace_back();
					vi.type = type;
					vi.name = name;
					vi.offset = offset;
					vi.array_size = arr_size;
					vi.array_stride = arr_stride;
				}

				db.udts.emplace(sh(name.c_str()), ui);
			}
			else if (src.basetype == spirv_cross::SPIRType::Image || src.basetype == spirv_cross::SPIRType::SampledImage)
				ret = TypeInfo::get(TagPU, "ShaderImage", db);
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
						case 1: ret = TypeInfo::get<int>();		break;
						case 2: ret = TypeInfo::get<ivec2>();	break;
						case 3: ret = TypeInfo::get<ivec3>();	break;
						case 4: ret = TypeInfo::get<ivec4>();	break;
						default: assert(0);
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
						case 1: ret = TypeInfo::get<uint>();	break;
						case 2: ret = TypeInfo::get<uvec2>();	break;
						case 3: ret = TypeInfo::get<uvec3>();	break;
						case 4: ret = TypeInfo::get<uvec4>();	break;
						default: assert(0);
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
						case 1: ret = TypeInfo::get<float>();	break;
						case 2: ret = TypeInfo::get<vec2>();	break;
						case 3: ret = TypeInfo::get<vec3>();	break;
						case 4: ret = TypeInfo::get<vec4>();	break;
						default: assert(0);
						}
						break;
					case 2:
						switch (src.vecsize)
						{
						case 2: ret = TypeInfo::get<mat2>(); break;
						default: assert(0);
						}
						break;
					case 3:
						switch (src.vecsize)
						{
						case 3: ret = TypeInfo::get<mat3>(); break;
						default: assert(0);
						}
						break;
					case 4:
						switch (src.vecsize)
						{
						case 4: ret = TypeInfo::get<mat4>(); break;
						default: assert(0);
						}
						break;
					default:
						assert(0);
					}
					break;
				}
			}

			return ret;
		}

		bool compile_shader(ShaderStageFlags stage, const std::filesystem::path& src_path, const std::vector<std::string>& defines, const std::filesystem::path& dst_path)
		{
			if (std::filesystem::exists(dst_path))
			{
				auto dst_date = std::filesystem::last_write_time(dst_path);
				if (dst_date > std::filesystem::last_write_time(src_path))
				{
					std::vector<std::filesystem::path> dependencies;

					std::ifstream file(dst_path);
					LineReader dst(file);
					dst.read_block("dependencies:");
					unserialize_text(dst, &dependencies);
					file.close();

					auto up_to_date = true;
					for (auto& d : dependencies)
					{
						if (std::filesystem::last_write_time(d) > dst_date)
						{
							up_to_date = false;
							break;
						}
					}
					if (up_to_date)
						return false;
				}
			}

			auto dst_ppath = dst_path.parent_path();
			if (!dst_ppath.empty() && !std::filesystem::exists(dst_ppath))
				std::filesystem::create_directories(dst_ppath);

			std::ofstream dst(dst_path);

			{
				std::vector<std::filesystem::path> dependencies;
				std::list<std::filesystem::path> headers;
				headers.push_back(src_path);
				while (!headers.empty())
				{
					auto fn = headers.front();
					if (!std::filesystem::exists(fn))
					{
						printf("cannot find include file: %s\n", fn.string().c_str());
						continue;
					}

					fn = std::filesystem::canonical(fn);
					fn.make_preferred();
					headers.pop_front();

					if (std::find(dependencies.begin(), dependencies.end(), fn) == dependencies.end())
						dependencies.push_back(fn);

					std::ifstream file(fn);
					auto ppath = fn.parent_path();
					while (!file.eof())
					{
						std::string line;
						std::getline(file, line);
						if (!line.empty() && line[0] != '#')
							break;
						if (SUS::remove_both_ends(line, "#include \"", "\""))
						{
							std::filesystem::path p = line;
							if (!p.is_absolute())
								p = ppath / p;
							headers.push_back(p);
						}
					}
					file.close();
				}

				dst << "dependencies:" << std::endl;
				serialize_text(&dependencies, dst);
				dst << std::endl;
			}

			std::filesystem::path temp_path = L"temp.glsl";
			std::ofstream temp(temp_path);
			temp << "#version 450 core" << std::endl;
			temp << "#extension GL_ARB_shading_language_420pack : enable" << std::endl;
			temp << "#extension GL_ARB_separate_shader_objects : enable" << std::endl;
			temp << std::endl;
			for (auto& d : defines)
				temp << "#define " << d << std::endl;
			temp << std::endl;

			std::ifstream src(src_path);
			if (stage == ShaderStageDsl)
			{
				temp << "#define SET 0" << std::endl;
				while (!src.eof())
				{
					std::string line;
					std::getline(src, line);
					temp << line << std::endl;
				}
				temp << std::endl;
				temp << "void main() {}" << std::endl;
				temp << std::endl;
			}
			else if (stage == ShaderStagePll)
			{
				temp << "#define SET 0" << std::endl;
				while (!src.eof())
				{
					std::string line;
					std::getline(src, line);
					if (line.starts_with("#include \"") && line.ends_with(".dsl\""))
						continue;
					temp << line << std::endl;
				}
				temp << std::endl;
				temp << "void main() {}" << std::endl;
				temp << std::endl;
			}
			else
			{
				auto set = 0;
				temp << "#define SET " << std::to_string(set++) << std::endl;
				while (!src.eof())
				{
					std::string line;
					std::getline(src, line);
					if (SUS::remove_both_ends(line, "#include \"", "\""))
					{
						temp << std::endl;

						std::ifstream pll(src_path.parent_path() / line);
						while (!pll.eof())
						{
							std::getline(pll, line);
							temp << line << std::endl;
							if (line.starts_with("#include \"") && line.ends_with(".dsl\""))
							{
								temp << "#undef SET" << std::endl;
								temp << "#define SET " << std::to_string(set++) << std::endl;
							}
						}
						pll.close();

						continue;
					}
					temp << line << std::endl;
				}
			}
			src.close();

			temp.close();

			auto vk_sdk_path = getenv("VK_SDK_PATH");
			if (!vk_sdk_path)
			{
				printf("cannot find VulkanSDK\n");
				return false;
			}

			wprintf(L"compiling: %s\n", src_path.c_str());
			if (!defines.empty())
			{
				printf("   with defines: ");
				for (auto& d : defines)
					printf("%s ", d.c_str());
				printf("\n");
			}
			std::filesystem::remove(L"temp.spv");

			auto stage_str = get_stage_str(stage);
			std::string errors;
			exec(std::filesystem::path(vk_sdk_path) / L"Bin/glslc.exe", L" -fshader-stage=" + stage_str + L" -I \"" + src_path.parent_path().wstring() + L"\" " + temp_path.wstring() + L" -o temp.spv", &errors);
			if (!std::filesystem::exists(L"temp.spv"))
			{
				printf("%s\n", errors.c_str());
				shell_exec(temp_path.wstring(), L"", false, true);
				assert(0);
				return false;
			}
			printf(" - done\n");
			std::filesystem::remove(L"temp.glsl");

			auto spv = get_file_content(L"temp.spv");
			auto spv_array = std::vector<uint>(spv.size() / 4);
			memcpy(spv_array.data(), spv.data(), spv_array.size() * sizeof(uint));
			std::filesystem::remove(L"temp.spv");

			TypeInfoDataBase db;
			auto spv_compiler = spirv_cross::CompilerGLSL(spv_array.data(), spv_array.size());
			auto spv_resources = spv_compiler.get_shader_resources();

			if (stage == ShaderStageDsl || stage == ShaderStagePll)
			{
				std::vector<DescriptorBinding> bindings;

				auto get_binding = [&](spirv_cross::Resource& r, DescriptorType type) {
					get_shader_type(spv_compiler, r.base_type_id, db);

					auto binding = spv_compiler.get_decoration(r.id, spv::DecorationBinding);
					if (bindings.size() <= binding)
						bindings.resize(binding + 1);

					auto& b = bindings[binding];
					b.type = type;
					b.count = max(1U, spv_compiler.get_type(r.type_id).array[0]);
					b.name = spv_compiler.get_name(r.base_type_id);
				};

				for (auto& r : spv_resources.uniform_buffers)
					get_binding(r, DescriptorUniformBuffer);
				for (auto& r : spv_resources.storage_buffers)
					get_binding(r, DescriptorStorageBuffer);
				for (auto& r : spv_resources.sampled_images)
					get_binding(r, DescriptorSampledImage);
				for (auto& r : spv_resources.storage_images)
					get_binding(r, DescriptorStorageImage);

				dst << "dsl:" << std::endl;
				serialize_text(&bindings, dst);
				dst << std::endl;

				if (!spv_resources.push_constant_buffers.empty())
					get_shader_type(spv_compiler, spv_resources.push_constant_buffers[0].base_type_id, db);
			}
			else
			{
				dst << "spv:" << std::endl;
				for (auto i = 0; i < spv_array.size(); i++)
				{
					dst << to_hex_string(spv_array[i]) << " ";
					if (i % 10 == 9)
						dst << std::endl;
				}
				if (spv_array.size() % 10 != 0)
					dst << std::endl;
				dst << std::endl;

				if (stage == ShaderStageVert)
				{
					UdtInfo ui;
					ui.name = "Input";
					for (auto& r : spv_resources.stage_inputs)
					{
						auto location = spv_compiler.get_decoration(r.id, spv::DecorationLocation);
						auto& vi = ui.variables.emplace_back();
						vi.type = get_shader_type(spv_compiler, r.base_type_id, db);
						vi.name = r.name;
						if (vi.type == TypeInfo::get<vec4>())
						{
							if (vi.name.ends_with("_col") || vi.name.ends_with("_color"))
								vi.type = TypeInfo::get<cvec4>();
						}
						vi.offset = ui.size;
						{
							auto& m = vi.metas.d.emplace_back();
							m.first = "Location"_h;
							m.second.i = location;
						}
						ui.size += vi.type->size;
					}
					db.udts.emplace(sh(ui.name.c_str()), ui);
				}
			}

			dst << "typeinfo:" << std::endl;
			db.save(dst);
			dst << std::endl;

			dst.close();
			return true;
		}

		std::filesystem::path get_res_path(const std::filesystem::path& filename, const std::vector<std::string>& defines)
		{
			auto ret = filename;
			if (!defines.empty())
			{
				auto hash = 0U;
				for (auto& d : defines)
					hash = hash ^ std::hash<std::string>()(d);
				auto str_hash = to_hex_wstring(hash);
				ret += L"." + to_hex_wstring(hash);
			}
			ret += L".res";
			return ret;
		}

		DescriptorPoolPrivate::~DescriptorPoolPrivate()
		{
			vkDestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
		}

		struct DescriptorPoolCurrent : DescriptorPool::Current
		{
			DescriptorPoolPtr operator()(DevicePtr device) override
			{
				if (!device)
					device = current_device;

				return device->dsp.get();
			}
		}DescriptorPool_current;
		DescriptorPool::Current& DescriptorPool::current = DescriptorPool_current;


		struct DescriptorPoolCreate : DescriptorPool::Create
		{
			DescriptorPoolPtr operator()(DevicePtr device) override
			{
				if (!device)
					device = current_device;

				auto ret = new DescriptorPoolPrivate;
				ret->device = device;

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
				chk_res(vkCreateDescriptorPool(device->vk_device, &descriptorPoolInfo, nullptr, &ret->vk_descriptor_pool));

				return ret;
			}
		}DescriptorPool_create;
		DescriptorPool::Create& DescriptorPool::create = DescriptorPool_create;

		DescriptorSetLayoutPrivate::~DescriptorSetLayoutPrivate()
		{
			vkDestroyDescriptorSetLayout(device->vk_device, vk_descriptor_set_layout, nullptr);
		}

		DescriptorSetLayoutPtr DescriptorSetLayoutPrivate::load_from_res(DevicePtr device, const std::filesystem::path& filename)
		{
			if (!std::filesystem::exists(filename))
				return nullptr;

			std::vector<DescriptorBinding> bindings;
			TypeInfoDataBase db;

			std::ifstream file(filename);
			LineReader res(file);
			res.read_block("dsl:");
			unserialize_text(res, &bindings);
			res.read_block("typeinfo:", "");
			db.load(file);
			file.close();

			auto ret = DescriptorSetLayout::create(device, bindings);
			ret->db = std::move(db);
			for (auto& b : ret->bindings)
			{
				if (b.type == DescriptorUniformBuffer || b.type == DescriptorStorageBuffer)
					b.ui = find_udt(sh(b.name.c_str()), db);
			}
			return ret;
		}

		struct DescriptorSetLayoutCreate : DescriptorSetLayout::Create
		{
			DescriptorSetLayoutPtr operator()(DevicePtr device, std::span<DescriptorBinding> bindings) override
			{
				if (!device)
					device = current_device;

				auto ret = new DescriptorSetLayoutPrivate;
				ret->device = device;
				ret->bindings.assign(bindings.begin(), bindings.end());

				std::vector<VkDescriptorSetLayoutBinding> vk_bindings(bindings.size());
				for (auto i = 0; i < bindings.size(); i++)
				{
					auto& src = bindings[i];
					auto& dst = vk_bindings[i];

					dst.binding = i;
					dst.descriptorType = to_backend(src.type);
					dst.descriptorCount = src.count;
					dst.stageFlags = to_backend_flags<ShaderStageFlags>(ShaderStageAll);
					dst.pImmutableSamplers = nullptr;
				}

				VkDescriptorSetLayoutCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				info.flags = 0;
				info.pNext = nullptr;
				info.bindingCount = vk_bindings.size();
				info.pBindings = vk_bindings.data();

				chk_res(vkCreateDescriptorSetLayout(device->vk_device, &info, nullptr, &ret->vk_descriptor_set_layout));

				return ret;
			}

			DescriptorSetLayoutPtr operator()(DevicePtr device, const std::string& content, const std::filesystem::path& dst, const std::filesystem::path& src) override
			{
				if (!device)
					device = current_device;

				auto fn = dst;
				if (dst.empty())
					fn = L"#temp.res";

				std::filesystem::path temp_path = L"temp.dsl";
				std::ofstream file(temp_path);
				file << content;
				file.close();
				if (!src.empty())
					std::filesystem::last_write_time(temp_path, std::filesystem::last_write_time(src));

				compile_shader(ShaderStageDsl, temp_path, {}, fn);
				std::filesystem::remove(temp_path);

				auto ret = DescriptorSetLayoutPrivate::load_from_res(device, fn);
				if (!ret)
					return nullptr;
				if (fn.c_str()[0] == L'#')
					std::filesystem::remove(fn);
				else
				{
					auto str = fn.wstring();
					if (auto p = str.find('#'); p != std::wstring::npos)
						fn = str.substr(0, p);
				}
				ret->filename = fn;
				return ret;
			}
		}DescriptorSetLayout_create;
		DescriptorSetLayout::Create& DescriptorSetLayout::create = DescriptorSetLayout_create;

		struct DescriptorSetLayoutGet : DescriptorSetLayout::Get
		{
			DescriptorSetLayoutPtr operator()(DevicePtr device, const std::filesystem::path& _filename) override
			{
				if (!device)
					device = current_device;

				auto filename = Path::get(_filename);

				if (device)
				{
					for (auto& d : device->dsls)
					{
						if (d->filename.filename() == filename)
							return d.get();
					}
				}

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find dsl: %s\n", _filename.c_str());
					return nullptr;
				}

				auto res_path = filename;
				res_path += L".res";
				compile_shader(ShaderStageDsl, filename, {}, res_path);

				if (device)
				{
					auto ret = DescriptorSetLayoutPrivate::load_from_res(device, res_path);
					if (ret)
					{
						ret->filename = filename;
						device->dsls.emplace_back(ret);
						return ret;
					}
				}
				return nullptr;
			}
		}DescriptorSetLayout_get;
		DescriptorSetLayout::Get& DescriptorSetLayout::get = DescriptorSetLayout_get;

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

		struct DescriptorSetCreate : DescriptorSet::Create
		{
			DescriptorSetPtr operator()(DescriptorPoolPtr pool, DescriptorSetLayoutPtr layout) override
			{
				if (!pool)
					pool = DescriptorPool::current();

				auto ret = new DescriptorSetPrivate;
				ret->device = pool->device;
				ret->pool = pool;
				ret->layout = layout;

				ret->reses.resize(layout->bindings.size());
				for (auto i = 0; i < ret->reses.size(); i++)
					ret->reses[i].resize(max(1U, layout->bindings[i].count));

				VkDescriptorSetAllocateInfo info;
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				info.pNext = nullptr;
				info.descriptorPool = pool->vk_descriptor_pool;
				info.descriptorSetCount = 1;
				info.pSetLayouts = &layout->vk_descriptor_set_layout;

				chk_res(vkAllocateDescriptorSets(pool->device->vk_device, &info, &ret->vk_descriptor_set));

				return ret;
			}
		}DescriptorSet_create;
		DescriptorSet::Create& DescriptorSet::create = DescriptorSet_create;

		PipelineLayoutPrivate::~PipelineLayoutPrivate()
		{
			if (!descriptor_set_layouts.empty() && descriptor_set_layouts.back()->filename == filename)
				delete descriptor_set_layouts.back();
			vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
		}

		PipelineLayoutPtr PipelineLayoutPrivate::load_from_res(DevicePtr device, const std::filesystem::path& filename)
		{
			if (!std::filesystem::exists(filename))
				return nullptr;

			std::vector<std::filesystem::path> dependencies;
			std::vector<DescriptorBinding> bindings;
			TypeInfoDataBase db;

			std::ifstream file(filename);
			LineReader res(file);
			res.read_block("dependencies:");
			unserialize_text(res, &dependencies);
			res.read_block("dsl:");
			unserialize_text(res, &bindings);
			res.read_block("typeinfo:", "");
			db.load(file);
			file.close();

			std::vector<DescriptorSetLayoutPrivate*> dsls;

			for (auto& d : dependencies)
			{
				if (d.extension() == L".dsl")
					dsls.push_back(DescriptorSetLayout::get(device, d));
			}
			if (!bindings.empty())
			{
				auto dsl = DescriptorSetLayout::create(device, bindings);
				dsl->filename = filename;
				dsls.push_back(dsl);
			}

			auto pc_ui = find_udt("PushConstant"_h, db);

			auto ret = PipelineLayout::create(device, dsls, pc_ui ? pc_ui->size : 0);
			ret->db = std::move(db);
			ret->pc_ui = pc_ui;
			return ret;
		}

		struct PipelineLayoutCreate : PipelineLayout::Create
		{
			PipelineLayoutPtr operator()(DevicePtr device, std::span<DescriptorSetLayoutPtr> descriptor_set_layouts, uint push_constant_size) override
			{
				if (!device)
					device = current_device;

				auto ret = new PipelineLayoutPrivate;
				ret->device = device;
				ret->descriptor_set_layouts.assign(descriptor_set_layouts.begin(), descriptor_set_layouts.end());
				ret->pc_sz = push_constant_size;

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

				chk_res(vkCreatePipelineLayout(device->vk_device, &info, nullptr, &ret->vk_pipeline_layout));

				return ret;
			}

			PipelineLayoutPtr operator()(DevicePtr device, const std::string& content, const std::filesystem::path& dst, const std::filesystem::path& src) override
			{
				if (!device)
					device = current_device;

				auto fn = dst;
				if (dst.empty())
					fn = L"#temp.res";

				std::filesystem::path temp_path = L"temp.pll";
				std::ofstream file(temp_path);
				file << content;
				file.close();
				if (!src.empty())
					std::filesystem::last_write_time(temp_path, std::filesystem::last_write_time(src));

				compile_shader(ShaderStagePll, temp_path, {}, fn);
				std::filesystem::remove(temp_path);

				auto ret = PipelineLayoutPrivate::load_from_res(device, fn);
				if (!ret)
					return nullptr;
				if (fn.c_str()[0] == L'#')
					std::filesystem::remove(fn);
				else
				{
					auto str = fn.wstring();
					if (auto p = str.find('#'); p != std::wstring::npos)
						fn = str.substr(0, p);
				}
				ret->filename = fn;
				return ret;
			}
		}PipelineLayout_create;
		PipelineLayout::Create& PipelineLayout::create = PipelineLayout_create;

		struct PipelineLayoutGet : PipelineLayout::Get
		{
			PipelineLayoutPtr operator()(DevicePtr device, const std::filesystem::path& _filename) override
			{
				if (!device)
					device = current_device;

				auto filename = Path::get(_filename);

				if (device)
				{
					for (auto& p : device->plls)
					{
						if (p->filename == filename)
							return p.get();
					}
				}

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find pll: %s\n", _filename.c_str());
					return nullptr;
				}

				auto res_path = filename;
				res_path += L".res";
				compile_shader(ShaderStagePll, filename, {}, res_path);

				auto ret = PipelineLayoutPrivate::load_from_res(device, res_path);
				if (ret)
				{
					ret->filename = filename;
					device->plls.emplace_back(ret);
					return ret;
				}
				return nullptr;
			}
		}PipelineLayout_get;
		PipelineLayout::Get& PipelineLayout::get = PipelineLayout_get;

		ShaderPrivate::~ShaderPrivate()
		{
			if (vk_module)
				vkDestroyShaderModule(device->vk_device, vk_module, nullptr);
		}

		ShaderPtr ShaderPrivate::load_from_res(DevicePtr device, const std::filesystem::path& filename)
		{
			if (!std::filesystem::exists(filename))
				return nullptr;

			std::vector<uint> spv;
			TypeInfoDataBase db;

			std::ifstream file(filename);
			LineReader res(file);
			res.read_block("spv:");
			for (auto& l : res.lines)
			{
				for (auto& b : SUS::split(l))
					spv.push_back(from_hex_string(b));
			}
			res.read_block("typeinfo:", "");
			db.load(file);
			file.close();

			auto ret = new ShaderPrivate;
			ret->device = device;
			ret->db = std::move(db);
			ret->in_ui = find_udt("Input"_h, ret->db);
			ret->out_ui = find_udt("Output"_h, ret->db);

			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = spv.size() * sizeof(uint);
			shader_info.pCode = spv.data();
			chk_res(vkCreateShaderModule(device->vk_device, &shader_info, nullptr, &ret->vk_module));

			return ret;
		}

		struct ShaderCreate : Shader::Create
		{
			ShaderPtr operator()(DevicePtr device, ShaderStageFlags type, const std::string& content, const std::filesystem::path& dst, const std::filesystem::path& src) override
			{
				if (!device)
					device = current_device;

				auto fn = dst;
				if (dst.empty())
					fn = L"#temp.res";

				std::filesystem::path temp_path = L"temp." + get_stage_str(type);
				std::ofstream file(temp_path);
				file << content;
				file.close();
				if (!src.empty())
					std::filesystem::last_write_time(temp_path, std::filesystem::last_write_time(src));

				compile_shader(type, temp_path, {}, fn);
				std::filesystem::remove(temp_path);

				auto ret = ShaderPrivate::load_from_res(device, fn);
				if (!ret)
					return nullptr;
				if (fn.c_str()[0] == L'#')
					std::filesystem::remove(fn);
				else
				{
					auto str = fn.wstring();
					if (auto p = str.find('#'); p != std::wstring::npos)
						fn = str.substr(0, p);
				}
				ret->type = type;
				ret->filename = fn;
				return ret;
			}
		}Shader_create;
		Shader::Create& Shader::create = Shader_create;

		struct ShaderGet : Shader::Get
		{
			ShaderPtr operator()(DevicePtr device, ShaderStageFlags type, const std::filesystem::path& _filename, const std::vector<std::string>& defines) override
			{
				if (!device)
					device = current_device;

				auto filename = Path::get(_filename);

				if (device)
				{
					for (auto& s : device->sds)
					{
						if (s->filename == filename && s->defines == defines)
							return s.get();
					}
				}

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find shader: %s\n", _filename.c_str());
					return nullptr;
				}

				if (type == ShaderStageNone)
					type = stage_from_ext(filename);

				auto res_path = get_res_path(filename, defines);
				compile_shader(type, filename, defines, res_path);

				if (device)
				{
					auto ret = ShaderPrivate::load_from_res(device, res_path);
					ret->type = type;
					ret->filename = filename;
					ret->defines = defines;
					device->sds.emplace_back(ret);
					return ret;
				}

				return nullptr;
			}
		}Shader_get;
		Shader::Get& Shader::get = Shader_get;

		GraphicsPipelinePrivate::~GraphicsPipelinePrivate()
		{
			if (!filename.empty())
			{
				for (auto s : info.shaders)
				{
					if (s->filename == filename)
						delete s;
				}
				if (info.layout->filename == filename)
					delete info.layout;
				if (info.renderpass->filename == filename)
					delete info.renderpass;
			}
			vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
		}

		GraphicsPipelinePtr GraphicsPipelinePrivate::load(DevicePtr device, const std::filesystem::path& filename, const std::vector<std::string>& _defines)
		{
			auto ppath = filename.parent_path();

			GraphicsPipelineInfo info;

			std::ifstream file(filename);
			LineReader res(file);
			res.read_block("");

			std::string												layout_segment;
			std::vector<std::pair<ShaderStageFlags, std::string>>	shader_segments;

			std::vector<std::string>								renderpass_defines;
			std::vector<std::pair<ShaderStageFlags, std::string>>	shader_defines;
			std::vector<std::string>								pipeline_defines;
			for (auto& d : _defines)
			{
				auto dst_and_value = SUS::split(d, '=');
				auto scope_and_name = SUS::split(dst_and_value[0], ':');
				if (scope_and_name.size() == 1)
					pipeline_defines.push_back(d);
				else
				{
					auto sp = SUS::split(scope_and_name[0], '|');
					for (auto& s : sp)
					{
						auto form_define = [&]() {
							auto ret = scope_and_name[1];
							if (dst_and_value.size() > 1)
							{
								ret += '=';
								ret += dst_and_value[1];
							}
							return ret;
						};
						if		(s == "rp")
							renderpass_defines.push_back(form_define());
						else if (s == "vert")
							shader_defines.emplace_back(ShaderStageVert, form_define());
						else if (s == "tesc")
							shader_defines.emplace_back(ShaderStageTesc, form_define());
						else if (s == "tese")
							shader_defines.emplace_back(ShaderStageTese, form_define());
						else if (s == "geom")
							shader_defines.emplace_back(ShaderStageGeom, form_define());
						else if (s == "frag")
							shader_defines.emplace_back(ShaderStageFrag, form_define());
						else if (s == "all_shader")
							shader_defines.emplace_back(ShaderStageAll, form_define());
					}
				}
			}

			UnserializeTextSpec spec;
			spec.map[TypeInfo::get<PipelineLayout*>()] = [&](const TextSerializeNode& src)->void* {
				auto value = src.value();
				if (value.starts_with("0x"))
					return (void*)sto<uint64>(value.substr(2));
				if (value.starts_with("@"))
				{
					layout_segment = value;
					return INVALID_POINTER;
				}
				std::filesystem::path fn = src.value();
				if (!ppath.empty() && Path::cat_if_in(ppath, fn))
					fn = std::filesystem::canonical(fn);
				return PipelineLayout::get(device, fn);
			};
			spec.map[TypeInfo::get<Shader*>()] = [&](const TextSerializeNode& src)->void* {
				auto value = src.value();
				if (!value.empty())
				{
					if (value.starts_with("@"))
					{
						ShaderStageFlags type = ShaderStageNone;
						if (value ==		"@vert")
							type = ShaderStageVert;
						else if (value ==	"@tesc")
							type = ShaderStageTesc;
						else if (value ==	"@tese")
							type = ShaderStageTese;
						else if (value ==	"@geom")
							type = ShaderStageGeom;
						else if (value ==	"@frag")
							type = ShaderStageFrag;
						if (type != ShaderStageNone)
							shader_segments.emplace_back(type, value);
						return INVALID_POINTER;
					}
					std::filesystem::path fn = value;
					auto stage = stage_from_ext(fn);
					if (stage != ShaderStageNone)
					{
						std::vector<std::string> defines;
						for (auto& d : shader_defines)
						{
							if (d.first == ShaderStageAll || d.first == stage)
								defines.push_back(d.second);
						}
						std::sort(defines.begin(), defines.end());
						return Shader::get(device, stage, value, defines);
					}
				}
				std::filesystem::path fn = src.value("filename");
				if (!ppath.empty() && Path::cat_if_in(ppath, fn))
					fn = std::filesystem::canonical(fn);
				auto stage = stage_from_ext(fn);
				if (stage != ShaderStageNone)
				{
					auto defines = format_defines(src.value("defines"));
					for (auto& d : shader_defines)
					{
						if (d.first == ShaderStageAll || d.first == stage)
							defines.push_back(d.second);
					}
					std::sort(defines.begin(), defines.end());
					return Shader::get(device, stage, fn, defines);
				}
				return INVALID_POINTER;
			};
			spec.map[TypeInfo::get<Renderpass*>()] = [&](const TextSerializeNode& src)->void* {
				auto value = src.value();
				if (value.starts_with("0x"))
					return (void*)sto<uint64>(value.substr(2));
				if (!value.empty())
				{
					auto defines = renderpass_defines;
					std::sort(defines.begin(), defines.end());
					return Renderpass::get(device, value, defines);
				}
				std::filesystem::path fn = src.value("filename");
				if (!ppath.empty() && Path::cat_if_in(ppath, fn))
					fn = std::filesystem::canonical(fn);
				auto defines = format_defines(src.value("defines"));
				defines.insert(defines.end(), renderpass_defines.begin(), renderpass_defines.end());
				std::sort(defines.begin(), defines.end());
				return Renderpass::get(device, fn, defines);
			};
			auto defines = pipeline_defines;
			std::sort(defines.begin(), defines.end());
			unserialize_text(res, &info, spec, defines);

			if (!layout_segment.empty())
			{
				res.read_block(layout_segment, "@");
				layout_segment = res.form_content();
			}
			for (auto& s : shader_segments)
			{
				res.read_block(s.second, "@");
				s.second = res.form_content();
				if (!layout_segment.empty())
					s.second = layout_segment + "\n\n" + s.second;
			}

			file.close();

			static int create_id = 0;
			if (filename.empty())
				create_id++;

			if (!layout_segment.empty())
			{
				info.layout = PipelineLayout::create(device, layout_segment,
					!filename.empty() ? filename.wstring() + L"#pll.res" : L"#" + std::to_wstring(create_id), filename);
			}
			for (auto& s : shader_segments)
			{
				info.shaders.push_back(Shader::create(device, s.first, s.second,
					!filename.empty() ? filename.wstring() + (L"#" + get_stage_str(s.first) + L".res") : L"#" + std::to_wstring(create_id), filename));
			}

			if (info.vertex_buffers.empty())
			{
				for (auto s : info.shaders)
				{
					if (s->type == ShaderStageVert)
					{
						if (s->in_ui && !s->in_ui->variables.empty())
						{
							auto& vb = info.vertex_buffers.emplace_back();
							for (auto& vi : s->in_ui->variables)
							{
								auto& va = vb.attributes.emplace_back();
								LightCommonValue cv;
								vi.metas.get("Location"_h, &cv);
								va.location = cv.i;
								if (vi.type == TypeInfo::get<float>())
									va.format = Format_R32_SFLOAT;
								else if (vi.type == TypeInfo::get<vec2>())
									va.format = Format_R32G32_SFLOAT;
								else if (vi.type == TypeInfo::get<vec3>())
									va.format = Format_R32G32B32_SFLOAT;
								else if (vi.type == TypeInfo::get<vec4>())
									va.format = Format_R32G32B32A32_SFLOAT;
								else if (vi.type == TypeInfo::get<cvec4>())
									va.format = Format_R8G8B8A8_UNORM;
							}
						}
						break;
					}
				}
			}

			return GraphicsPipeline::create(device, info);
		}

		struct GraphicsPipelineCreate : GraphicsPipeline::Create
		{
			GraphicsPipelinePtr operator()(DevicePtr device, const GraphicsPipelineInfo& info) override
			{
				if (!device)
					device = current_device;

				auto ret = new GraphicsPipelinePrivate;
				ret->device = device;
				ret->info = info;

				std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
				std::vector<VkVertexInputAttributeDescription> vk_vi_attributes;
				std::vector<VkVertexInputBindingDescription> vk_vi_bindings;
				std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
				std::vector<VkDynamicState> vk_dynamic_states;

				vk_stage_infos.resize(info.shaders.size());
				for (auto i = 0; i < info.shaders.size(); i++)
				{
					auto shader = info.shaders[i];

					auto& dst = vk_stage_infos[i];
					dst.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					dst.flags = 0;
					dst.pNext = nullptr;
					dst.pSpecializationInfo = nullptr;
					dst.pName = "main";
					dst.stage = to_backend(shader->type);
					dst.module = shader->vk_module;
				}

				vk_vi_bindings.resize(info.vertex_buffers.size());
				for (auto i = 0; i < vk_vi_bindings.size(); i++)
				{
					auto& src_buf = info.vertex_buffers[i];
					auto& dst_buf = vk_vi_bindings[i];
					dst_buf.binding = i;
					auto offset = 0;
					for (auto j = 0; j < src_buf.attributes.size(); j++)
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
					auto& res_atts = info.renderpass->info.subpasses[info.subpass_index].resolve_attachments;
					multisample_state.rasterizationSamples = to_backend(!res_atts.empty() ? info.renderpass->info.attachments[res_atts[0]].sample_count : SampleCount_1);
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

				vk_blend_attachment_states.resize(info.renderpass->info.subpasses[info.subpass_index].color_attachments.size());
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
				for (auto i = 0; i < info.blend_options.size(); i++)
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

				for (auto i = 0; i < info.dynamic_states.size(); i++)
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
				pipeline_info.layout = info.layout->vk_pipeline_layout;
				pipeline_info.renderPass = info.renderpass->vk_renderpass;
				pipeline_info.subpass = info.subpass_index;
				pipeline_info.basePipelineHandle = 0;
				pipeline_info.basePipelineIndex = 0;

				chk_res(vkCreateGraphicsPipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &ret->vk_pipeline));

				return ret;
			}

			GraphicsPipelinePtr operator()(DevicePtr device, const std::string& content) override
			{
				if (!device)
					device = current_device;

				std::filesystem::path fn = L"#temp.pipeline";

				std::ofstream file(fn);
				file << SUS::get_ltrimed(content);
				file.close();

				auto ret = GraphicsPipelinePrivate::load(device, fn, {});
				std::filesystem::remove(fn);
				return ret;
			}
		}GraphicsPipeline_create;
		GraphicsPipeline::Create& GraphicsPipeline::create = GraphicsPipeline_create;

		struct GraphicsPipelineGet : GraphicsPipeline::Get
		{
			GraphicsPipelinePtr operator()(DevicePtr device, const std::filesystem::path& _filename, const std::vector<std::string>& defines) override
			{
				if (!device)
					device = current_device;

				auto filename = Path::get(_filename);

				if (device)
				{
					for (auto& pl : device->gpls)
					{
						if (pl->filename == filename && pl->defines == defines)
							return pl.get();
					}
				}

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find pipeline: %s\n", _filename.c_str());
					return nullptr;
				}

				if (device)
				{
					auto ret = GraphicsPipelinePrivate::load(device, filename, defines);
					ret->filename = filename;
					ret->defines = defines;
					device->gpls.emplace_back(ret);
					return ret;
				}

				return nullptr;
			}
		}GraphicsPipeline_get;
		GraphicsPipeline::Get& GraphicsPipeline::get = GraphicsPipeline_get;

		ComputePipelinePrivate::~ComputePipelinePrivate()
		{
			vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
		}

		struct ComputePipelineCreate : ComputePipeline::Create
		{
			ComputePipelinePtr operator()(DevicePtr device, const ComputePipelineInfo& info) override
			{
				if (!device)
					device = current_device;

				auto ret = new ComputePipelinePrivate;
				ret->device = device;
				ret->info = info;

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
				pipeline_info.stage.module = info.shader->vk_module;

				pipeline_info.basePipelineHandle = 0;
				pipeline_info.basePipelineIndex = 0;
				pipeline_info.layout = info.layout->vk_pipeline_layout;

				chk_res(vkCreateComputePipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &ret->vk_pipeline));

				return ret;
			}

			ComputePipelinePtr operator()(DevicePtr device, const std::string& content, const std::vector<std::string>& defines, const std::string& key) override
			{
				return nullptr;
			}
		}ComputePipeline_create;
		ComputePipeline::Create& ComputePipeline::create = ComputePipeline_create;

		struct ComputePipelineGet : ComputePipeline::Get
		{
			ComputePipelinePtr operator()(DevicePtr device, const std::filesystem::path& filename, const std::vector<std::string>& defines) override
			{
				return nullptr;
			}
		}ComputePipeline_get;
		ComputePipeline::Get& ComputePipeline::get = ComputePipeline_get;
	}
}
