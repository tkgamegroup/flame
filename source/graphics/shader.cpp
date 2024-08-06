#include "../serialize_extension.h"
#include "../foundation/typeinfo.h"
#include "../foundation/typeinfo_serialize.h"
#include "../foundation/system.h"
#include "device_private.h"
#include "command_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "material_private.h"
#include "shader_private.h"

#include <spirv_glsl.hpp>

namespace flame
{
	namespace graphics
	{
		std::unique_ptr<DescriptorPoolT>					descriptorset_pool;
		std::vector<std::unique_ptr<DescriptorSetLayoutT>>	loaded_descriptorsetlayouts;
		std::vector<std::unique_ptr<PipelineLayoutT>>		loaded_pipelinelayouts;
		std::vector<ShaderPtr>								shaders;
		std::vector<std::unique_ptr<ShaderT>>				loaded_shaders;
		std::vector<GraphicsPipelinePtr>					graphics_pipelines;
		std::vector<std::unique_ptr<GraphicsPipelineT>>		loaded_graphics_pipelines;
		std::vector<std::unique_ptr<ComputePipelineT>>		loaded_compute_pipelines;

		std::wstring get_stage_str(ShaderStageFlags stage)
		{
			switch (stage)
			{
			case ShaderStageVert:
			case ShaderVi:
				return L"vert";
			case ShaderStageTesc:
				return L"tesc";
			case ShaderStageTese:
				return L"tese";
			case ShaderStageGeom:
				return L"geom";
			case ShaderStageFrag:
			case ShaderDsl:
			case ShaderPll:
				return L"frag";
			case ShaderStageComp:
				return L"comp";
			case ShaderStageTask:
				return L"task";
			case ShaderStageMesh:
				return L"mesh";
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
			else if (ext == L".task")
				return ShaderStageTask;
			else if (ext == L".mesh")
				return ShaderStageMesh;
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

				for (auto i = 0; i < src.member_types.size(); i++)
				{
					auto id = src.member_types[i];

					auto type = get_shader_type(compiler, id, db);
					auto name = compiler.get_member_name(src.self, i);
					auto offset = compiler.type_struct_member_offset(src, i);
					auto array_size = compiler.get_type(id).array[0];
					auto array_stride = compiler.get_decoration(id, spv::DecorationArrayStride);
					if (array_stride == 0)
						array_size = 1;
					if (array_size > 1)
					{
						std::string name = std::format("{}[{}]", type->name, array_size);
						if (array_stride != type->size)
							name += std::format(":{}", array_stride);
						if (type->tag == TagU)
							type = TypeInfo::get(TagAU, name, db);
						else
							type = TypeInfo::get(TagAD, name, db);
					}
					auto& vi = ui.variables.emplace_back();
					vi.type = type;
					vi.name = name;
					vi.offset = offset;
				}

				db.udts.emplace(sh(name.c_str()), ui);

				ret = TypeInfo::get(TagU, name, db);
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
				case spirv_cross::SPIRType::UByte:
					switch (src.columns)
					{
					case 1:
						switch (src.vecsize)
						{
						case 1: ret = TypeInfo::get<uchar>();	break;
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
				}
			}

			assert(ret);
			return ret;
		}

		bool compile_shader(ShaderStageFlags stage, const std::filesystem::path& src_path, const std::vector<std::string>& _defines, const std::filesystem::path& dst_path)
		{
			static bool first = true;
			if (first)
			{
				first = false;

				if (auto flame_path = getenv("FLAME_PATH"); flame_path)
				{
					auto material_header_path = std::filesystem::path(flame_path) / L"source/graphics/material.h";
					auto defines_glsl_path = Path::get(L"flame/shaders/defines.glsl");
					if (!std::filesystem::exists(defines_glsl_path) || std::filesystem::last_write_time(defines_glsl_path) < std::filesystem::last_write_time(material_header_path))
					{
						if (auto ei = find_enum(th<MaterialFlags>()); ei)
						{
							std::ofstream file(defines_glsl_path);
							file << "//THIS FILE IS AUTO GENERATED\n";
							for (auto& i : ei->items)
								file << std::format("const uint MaterialFlag{}={};\n", i.name, i.value);
							file.close();
						}
					}
				}
			}

			if (std::filesystem::exists(dst_path))
			{
				auto dst_date = std::filesystem::last_write_time(dst_path);
				if (dst_date > std::filesystem::last_write_time(src_path))
				{
					std::vector<std::filesystem::path> dependencies;

					std::ifstream file(dst_path);
					LineReader res(file);
					res.read_block("dependencies:");
					unserialize_text(res, &dependencies);
					file.close();

					auto up_to_date = true;
					for (auto& d : dependencies)
					{
						if (!std::filesystem::exists(d) || std::filesystem::last_write_time(d) > dst_date)
						{
							up_to_date = false;
							break;
						}
					}
					if (up_to_date)
						return true;
				}
			}

			auto dst_ppath = dst_path.parent_path();
			if (!dst_ppath.empty() && !std::filesystem::exists(dst_ppath))
				std::filesystem::create_directories(dst_ppath);

			uint u;
			auto use_mesh_shader = device->get_config("mesh_shader"_h, u) ? u == 1 : true;

			std::string temp_content;
			std::vector<std::string> additional_lines;
			if (use_mesh_shader)
				temp_content += "#version 460\n";
			else
				temp_content += "#version 450\n";
			temp_content += "#extension GL_ARB_shading_language_420pack : enable\n";
			temp_content += "#extension GL_EXT_shader_8bit_storage: require\n";
			temp_content += "#extension GL_EXT_shader_explicit_arithmetic_types: require\n";
			temp_content += "#extension GL_EXT_shader_explicit_arithmetic_types_int8: require\n";
			if (stage == ShaderStageTask || stage == ShaderStageMesh)
			{
				temp_content += "#extension GL_EXT_mesh_shader : require\n";
				temp_content += "#extension GL_KHR_shader_subgroup_ballot : require\n";
				temp_content += "#extension GL_KHR_shader_subgroup_shuffle_relative : require\n";
			}
			temp_content += "\n";

			std::vector<std::pair<std::string, std::string>> defines;
			for (auto& d : _defines)
			{
				auto sp = SUS::split(d, '=');
				if (sp.front() == "__add_line__")
				{
					if (sp.size() > 1)
						additional_lines.push_back(std::string(sp.back()));
					continue;
				}
				defines.emplace_back(std::string(sp.front()), sp.size() > 1 ? std::string(sp.back()) : "");
			}
			for (auto& d : defines)
			{
				temp_content += "#define " + d.first;
				if (!d.second.empty())
					temp_content += " " + d.second;
				temp_content += "\n";
			}
			temp_content += "\n\n#define SET 0\n\n";

			std::vector<std::filesystem::path> dependencies;

			auto dsl_id = 0;
			std::function<std::string(const std::filesystem::path& path, const std::vector<std::string>& additional_lines)> preprocess;
			preprocess = [&](const std::filesystem::path& path, const std::vector<std::string>& additional_lines) {
				auto found_name = [&](std::string_view name) {
					for (auto& d : defines)
					{
						if (d.first == name)
							return true;
					}
					return false;
				};
				auto get_value = [&](const std::string& name)->std::string {
					for (auto& d : defines)
					{
						if (d.first == name)
							return d.second;
					}
					return name;
				};
				auto pass_condition = [&](const std::string& line) {
					static std::regex reg_exp(R"((\w+)\s*([<=>]+)\s*(\w+))");
					std::smatch res;
					if (std::regex_search(line, res, reg_exp))
					{
						auto op = res[2].str();
						if (op == "==")
							return get_value(res[1].str()) == get_value(res[3].str());
					}
					else
					{
						for (auto& d : defines)
						{
							if (d.first == line && s2t<uint>(d.second) != 0)
								return true;
						}

						if (s2t<uint>(line) != 0)
							return true;
					}
					return false;
				};

				std::vector<std::pair<bool, bool>> states; // first: current, second: the 'if' branch has taken
				auto ok = true;
				auto eval_state = [&]() {
					ok = true;
					for (auto& s : states)
						ok = ok && s.first;
				};
				auto in_loop = false;

				std::string ret;
				auto lines = get_file_lines(path);
				if (!additional_lines.empty())
					lines.insert(lines.begin(), additional_lines.begin(), additional_lines.end());
				for (auto& l : lines)
				{
					auto tl = SUS::get_trimed(l);
					if (SUS::strip_head_if(tl, "#define "))
					{
						if (ok)
						{
							auto sp = SUS::split_parentheses(tl, '(', ')', ' ');
							defines.emplace_back(std::string(sp.front()), sp.size() > 1 ? std::string(sp.back()) : "");
							ret += l;
							ret += "\n";
						}
					}
					else if (SUS::strip_head_if(tl, "#if "))
					{
						auto& s = states.emplace_back();
						s.first = pass_condition(tl);
						s.second = s.first;
						eval_state();
					}
					else if (SUS::strip_head_if(tl, "#ifdef "))
					{
						auto& s = states.emplace_back();
						s.first = found_name(tl);
						s.second = s.first;
						eval_state();
					}
					else if (SUS::strip_head_if(tl, "#ifndef "))
					{
						auto& s = states.emplace_back();
						s.first = !found_name(tl);
						s.second = s.first;
						eval_state();
					}
					else if (tl.starts_with("#else"))
					{
						auto& s = states.back();
						if (!s.second)
							s.first = !states.back().first;
						else
							s.first = false;
						s.second = true;
						eval_state();
					}
					else if (SUS::strip_head_if(tl, "#elif "))
					{
						auto& s = states.back();
						if (!s.second)
						{
							s.first = pass_condition(tl);
							if (s.first) s.second = true;
						}
						else
							s.first = false;
						eval_state();
					}
					else if (SUS::strip_head_if(tl, "#elifdef "))
					{
						auto& s = states.back();
						if (!s.second)
						{
							s.first = found_name(tl);
							if (s.first) s.second = true;
						}
						else
							s.first = false;
						eval_state();
					}
					else if (tl.starts_with("#endif"))
					{
						states.pop_back();
						eval_state();
					}
					else if (ok)
					{
						if (SUS::strip_head_if(tl, "#include "))
						{
							std::filesystem::path header_path;
							if (SUS::strip_head_tail_if(tl, "\"", "\""))
								header_path = tl;
							else if (SUS::strip_head_tail_if(tl, "<", ">"))
							{
								std::filesystem::path p(L"flame/shaders");
								p /= tl;
								header_path = Path::get(p);
							}
							else
							{
								for (auto& d : defines)
								{
									if (d.first == tl)
									{
										header_path = d.second;
										break;
									}
								}
							}
							if (!header_path.is_absolute())
							{
								header_path = path.parent_path() / header_path;
								assert(std::filesystem::exists(header_path));
								header_path = std::filesystem::canonical(header_path);
							}
							header_path.make_preferred();
							if (std::filesystem::exists(header_path))
							{
								if (std::find(dependencies.begin(), dependencies.end(), header_path) == dependencies.end())
									dependencies.push_back(header_path);
								auto is_dsl = header_path.extension() == L".dsl";
								if (!(is_dsl && stage == ShaderPll))
								{
									ret += preprocess(header_path, {});
									ret += "\n";
									if (is_dsl && stage != ShaderDsl)
									{
										ret += "#undef SET\n";
										dsl_id++;
										ret += std::format("#define SET {}\n\n", dsl_id);
									}
								}
							}
							else
							{
								wprintf(L"shader compile: cannot find header file: %s\n", header_path.c_str());
								assert(0);
							}
						}
						else
						{
							ret += l;
							ret += "\n";
						}
					}
				}
				return ret;
			};

			temp_content += preprocess(src_path, additional_lines);
			if (stage == ShaderVi || stage == ShaderDsl || stage == ShaderPll)
				temp_content += "void main() {}\n";
			std::ofstream dst(dst_path);
			dst << "dependencies:" << std::endl;
			serialize_text(&dependencies, dst);
			dst << std::endl;

			std::filesystem::path temp_path = L"temp.glsl";
			std::ofstream temp(temp_path);
			temp << temp_content;
			temp << std::endl;
			temp.close();

			std::filesystem::path glslc_path;
			if (auto vk_sdk_path = getenv("VK_SDK_PATH"); vk_sdk_path)
				glslc_path = std::format(L"{}/Bin/glslc.exe", s2w(vk_sdk_path));
			else
				glslc_path = L"glslc.exe";
			if (!std::filesystem::exists(glslc_path))
			{
				printf("cannot find shader compiler\n");
				return false;
			}

			wprintf(L"compiling shader: %s -> %s\n", src_path.c_str(), dst_path.c_str());
			if (!defines.empty())
			{
				printf("   with defines: ");
				for (auto& d : defines)
				{
					if (d.second.empty())
						printf("%s ", d.first.c_str());
					else
						printf("%s=%s ", d.first.c_str(), d.second.c_str());
				}
				printf("\n");
			}
			std::filesystem::remove(L"temp.spv");

			std::string errors;
			exec(glslc_path, std::format(L" --target-env=vulkan{} -fshader-stage={} -I \"{}\" {} -o temp.spv",
				use_mesh_shader ? L"1.3" : L"1.2", get_stage_str(stage), src_path.parent_path().wstring(), temp_path.wstring()), &errors);
			if (!std::filesystem::exists(L"temp.spv"))
			{
				printf("%s\n", errors.c_str());
				shell_exec(temp_path.wstring(), L"", false, true);
				dst.close();
				std::filesystem::remove(dst_path);
				assert(0);
				return false;
			}
			printf(" - done\n");
			std::filesystem::remove(L"temp.glsl");

			DataSoup spv;
			spv.load(L"temp.spv");
			std::filesystem::remove(L"temp.spv");

			TypeInfoDataBase db;
			auto spv_compiler = spirv_cross::CompilerGLSL((uint*)spv.soup.data(), spv.soup.size() / sizeof(uint));
			auto spv_resources = spv_compiler.get_shader_resources();

			if (stage == ShaderDsl || stage == ShaderPll)
			{
				std::vector<DescriptorBinding> bindings;

				auto get_binding = [&](spirv_cross::Resource& r, DescriptorType type) {
					get_shader_type(spv_compiler, r.base_type_id, db);

					auto binding = spv_compiler.get_decoration(r.id, spv::DecorationBinding);
					if (bindings.size() <= binding)
						bindings.resize(binding + 1);

					auto& b = bindings[binding];
					b.type = type;
					auto& spv_type = spv_compiler.get_type(r.type_id);
					b.count = spv_type.array.empty() ? 1 : spv_type.array[0];
					if (is_one_of(type, { DescriptorUniformBuffer, DescriptorStorageBuffer }))
						b.name = spv_compiler.get_name(r.base_type_id);
					else
						b.name = r.name;
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
				if (stage & ShaderStageAll)
				{
					dst << "spv:" << std::endl;
					spv.save(dst);
				}

				if (stage == ShaderStageVert || stage == ShaderVi)
				{
					UdtInfo ui;
					ui.name = "Input";
					std::vector<std::tuple<TypeInfo*, std::string, uint>> attributes;
					for (auto& r : spv_resources.stage_inputs)
					{
						auto location = spv_compiler.get_decoration(r.id, spv::DecorationLocation);
						auto ti = get_shader_type(spv_compiler, r.base_type_id, db);
						if (ti == TypeInfo::get<vec4>())
						{
							if (r.name.ends_with("_col") || r.name.ends_with("_color"))
								ti = TypeInfo::get<cvec4>();
						}
						attributes.emplace_back(ti, r.name, location);
					}
					std::sort(attributes.begin(), attributes.end(), [](const auto& a, const auto& b) {
						return std::get<2>(a) < std::get<2>(b);
					});
					for (auto& a : attributes)
					{
						auto& vi = ui.variables.emplace_back();
						vi.type = std::get<0>(a);
						vi.name = std::get<1>(a);
						vi.offset = ui.size;
						vi.metas.items.emplace("Location"_h, str(std::get<2>(a)));
						ui.size += vi.type->size;
					}
					db.udts.emplace(sh(ui.name.c_str()), ui);
				}
			}

			dst << "typeinfo:" << std::endl;
			dst << db.save_to_string();
			dst << std::endl;

			dst.close();
			return true;
		}

		std::wstring defines_to_hash_str(const std::vector<std::string>& defines)
		{
			std::wstring ret;
			if (!defines.empty())
			{
				auto hash = 0U;
				for (auto& d : defines)
					hash = hash ^ std::hash<std::string>()(d);
				auto str_hash = wstr_hex(hash);
				ret += L"." + wstr_hex(hash);
			}
			return ret;
		}

		DescriptorPoolPrivate::~DescriptorPoolPrivate()
		{
			if (app_exiting) return;

			vkDestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
			unregister_object(vk_descriptor_pool);
		}

		struct DescriptorPoolCurrent : DescriptorPool::Current
		{
			DescriptorPoolPtr operator()() override
			{
				return descriptorset_pool.get();
			}
		}DescriptorPool_current;
		DescriptorPool::Current& DescriptorPool::current = DescriptorPool_current;


		struct DescriptorPoolCreate : DescriptorPool::Create
		{
			DescriptorPoolPtr operator()() override
			{
				auto ret = new DescriptorPoolPrivate;

				VkDescriptorPoolSize descriptor_pool_sizes[] = {
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 512 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 8 },
				};

				VkDescriptorPoolCreateInfo descriptor_pool_info;
				descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptor_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				descriptor_pool_info.pNext = nullptr;
				descriptor_pool_info.poolSizeCount = countof(descriptor_pool_sizes);
				descriptor_pool_info.pPoolSizes = descriptor_pool_sizes;
				descriptor_pool_info.maxSets = 128;
				check_vk_result(vkCreateDescriptorPool(device->vk_device, &descriptor_pool_info, nullptr, &ret->vk_descriptor_pool));
				register_object(ret->vk_descriptor_pool, "Descriptor Pool", ret);

				return ret;
			}
		}DescriptorPool_create;
		DescriptorPool::Create& DescriptorPool::create = DescriptorPool_create;

		DescriptorSetLayoutPrivate::~DescriptorSetLayoutPrivate()
		{
			if (app_exiting) return;

			vkDestroyDescriptorSetLayout(device->vk_device, vk_descriptor_set_layout, nullptr);
			unregister_object(vk_descriptor_set_layout);
		}

		DescriptorSetLayoutPtr DescriptorSetLayoutPrivate::load_from_res(const std::filesystem::path& filename)
		{
			std::ifstream file(filename);
			if (!file.good())
				return nullptr;
			LineReader res(file);

			std::vector<DescriptorBinding> bindings;
			TypeInfoDataBase db;

			res.read_block("dsl:");
			unserialize_text(res, &bindings);
			res.read_block("typeinfo:");
			db.load_from_string(res.to_string());
			file.close();

			auto ret = DescriptorSetLayout::create(bindings);
			ret->db = std::move(db);
			for (auto& b : ret->bindings)
			{
				if (b.type == DescriptorUniformBuffer || b.type == DescriptorStorageBuffer)
					b.ui = find_udt(sh(b.name.c_str()), ret->db);
			}
			return ret;
		}

		struct DescriptorSetLayoutCreate : DescriptorSetLayout::Create
		{
			DescriptorSetLayoutPtr operator()(std::span<DescriptorBinding> bindings) override
			{
				auto ret = new DescriptorSetLayoutPrivate;
				ret->bindings.assign(bindings.begin(), bindings.end());

				std::vector<VkDescriptorSetLayoutBinding> vk_bindings(bindings.size());
				for (auto i = 0; i < bindings.size(); i++)
				{
					auto& src = bindings[i];
					auto& dst = vk_bindings[i];

					dst.binding = i;
					dst.descriptorType = to_vk(src.type);
					dst.descriptorCount = max(1U, src.count);
					dst.stageFlags = to_vk_flags<ShaderStageFlags>(ShaderStageAll);
					dst.pImmutableSamplers = nullptr;

					ret->bindings_map[sh(src.name.c_str())] = i;
				}

				VkDescriptorSetLayoutCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				info.flags = 0;
				info.pNext = nullptr;
				info.bindingCount = vk_bindings.size();
				info.pBindings = vk_bindings.data();

				check_vk_result(vkCreateDescriptorSetLayout(device->vk_device, &info, nullptr, &ret->vk_descriptor_set_layout));
				register_object(ret->vk_descriptor_set_layout, "Descriptor Set Layout", ret);

				return ret;
			}
		}DescriptorSetLayout_create;
		DescriptorSetLayout::Create& DescriptorSetLayout::create = DescriptorSetLayout_create;

		struct DescriptorSetLayoutGet : DescriptorSetLayout::Get
		{
			DescriptorSetLayoutPtr operator()(const std::filesystem::path& _filename) override
			{
				auto filename = Path::get(_filename);

				for (auto& d : loaded_descriptorsetlayouts)
				{
					if (d->filename.filename() == filename)
					{
						d->ref++;
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
				compile_shader(ShaderDsl, filename, {}, res_path);

				auto ret = DescriptorSetLayoutPrivate::load_from_res(res_path);
				if (ret)
				{
					ret->filename = filename;
					ret->ref = 1;
					loaded_descriptorsetlayouts.emplace_back(ret);
				}
				return ret;
			}
		}DescriptorSetLayout_get;
		DescriptorSetLayout::Get& DescriptorSetLayout::get = DescriptorSetLayout_get;

		struct DescriptorSetLayoutRelease : DescriptorSetLayout::Release
		{
			void operator()(DescriptorSetLayoutPtr dsl) override
			{
				if (dsl->ref == 1)
				{
					std::erase_if(loaded_descriptorsetlayouts, [&](const auto& i) {
						return i.get() == dsl;
					});
				}
				else
					dsl->ref--;
			}
		}DescriptorSetLayout_release;
		DescriptorSetLayout::Release& DescriptorSetLayout::release = DescriptorSetLayout_release;

		DescriptorSetPrivate::~DescriptorSetPrivate()
		{
			if (app_exiting) return;

			layout->ref--;

			check_vk_result(vkFreeDescriptorSets(device->vk_device, pool->vk_descriptor_pool, 1, &vk_descriptor_set));
			unregister_object(vk_descriptor_set);

			if (d3d12_descriptor_heap)
				d3d12_descriptor_heap->Release();
		}

		void DescriptorSetPrivate::set_buffer_i(uint binding, uint index, BufferPtr buf, uint offset, uint range)
		{
			if (binding >= reses.size() || index >= reses[binding].size())
				return;

			auto& res = reses[binding][index].b;
			if (res.vk_buf == buf->vk_buffer && res.offset == offset && res.range == range)
				return;

			res.vk_buf = buf->vk_buffer;
			res.offset = offset;
			res.range = range == 0 ? buf->size : range;

			buf_updates.emplace_back(binding, index);
		}

		void DescriptorSetPrivate::set_image_i(uint binding, uint index, ImageViewPtr iv, SamplerPtr sp)
		{
			if (binding >= reses.size() || index >= reses[binding].size())
				return;

			if (!sp)
				sp = Sampler::get(FilterLinear, FilterLinear, true, AddressClampToEdge);

			auto& res = reses[binding][index].i;
			if (res.vk_iv == iv->vk_image_view && res.vk_sp == sp->vk_sampler)
				return;

			res.vk_iv = iv->vk_image_view;
			res.vk_sp = sp->vk_sampler;

			img_updates.emplace_back(binding, index);
		}

		void DescriptorSetPrivate::update()
		{
			if (buf_updates.empty() && img_updates.empty())
				return;

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
				info.buffer = (VkBuffer)res.b.vk_buf;
				info.offset = res.b.offset;
				info.range = res.b.range;

				auto& wrt = vk_writes[idx];
				wrt.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wrt.pNext = nullptr;
				wrt.dstSet = vk_descriptor_set;
				wrt.dstBinding = u.first;
				wrt.dstArrayElement = u.second;
				wrt.descriptorType = to_vk(layout->bindings[u.first].type);
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
				auto type = layout->bindings[u.first].type;

				auto& info = vk_img_infos[i];
				info.imageView = (VkImageView)res.i.vk_iv;
				info.imageLayout = type == DescriptorSampledImage ?
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
				info.sampler = type == DescriptorSampledImage ? (VkSampler)res.i.vk_sp : nullptr;

				auto& wrt = vk_writes[idx];
				wrt.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wrt.pNext = nullptr;
				wrt.dstSet = vk_descriptor_set;
				wrt.dstBinding = u.first;
				wrt.dstArrayElement = u.second;
				wrt.descriptorType = to_vk(type);
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
				ret->pool = pool;
				ret->layout = layout;

				layout->ref++;

				ret->reses.resize(layout->bindings.size());
				for (auto i = 0; i < ret->reses.size(); i++)
					ret->reses[i].resize(max(1U, layout->bindings[i].count));

				VkDescriptorSetAllocateInfo info;
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				info.pNext = nullptr;
				info.descriptorPool = pool->vk_descriptor_pool;
				info.descriptorSetCount = 1;
				info.pSetLayouts = &layout->vk_descriptor_set_layout;

				check_vk_result(vkAllocateDescriptorSets(device->vk_device, &info, &ret->vk_descriptor_set));
				register_object(ret->vk_descriptor_set, "Descriptor Set", ret);

				{
					D3D12_DESCRIPTOR_HEAP_DESC desc = {};
					desc.NumDescriptors = ret->reses.size();
					desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
					desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
					check_dx_result(device->d3d12_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ret->d3d12_descriptor_heap)));
				}

				return ret;
			}
		}DescriptorSet_create;
		DescriptorSet::Create& DescriptorSet::create = DescriptorSet_create;

		PipelineLayoutPrivate::~PipelineLayoutPrivate()
		{
			if (app_exiting) return;

			for (auto dsl : dsls)
			{
				if (dsl->ref == 0)
					delete dsl;
			}

			vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
			unregister_object(vk_pipeline_layout);

			if (d3d12_signature)
				d3d12_signature->Release();
		}

		PipelineLayoutPtr PipelineLayoutPrivate::load_from_res(const std::filesystem::path& filename)
		{
			std::ifstream file(filename);
			if (!file.good())
				return nullptr;
			LineReader res(file);

			std::vector<std::filesystem::path> dependencies;
			std::vector<DescriptorBinding> bindings;
			TypeInfoDataBase db;

			res.read_block("dependencies:");
			unserialize_text(res, &dependencies);
			res.read_block("dsl:");
			unserialize_text(res, &bindings);
			res.read_block("typeinfo:");
			db.load_from_string(res.to_string());
			file.close();

			std::vector<DescriptorSetLayoutPrivate*> dsls;

			for (auto& d : dependencies)
			{
				if (d.extension() == L".dsl")
					dsls.push_back(DescriptorSetLayout::get(d));
			}
			if (!bindings.empty())
			{
				auto dsl = DescriptorSetLayout::create(bindings);
				auto str = filename.wstring();
				SUW::strip_tail_if(str, L".res");
				dsl->filename = str;
				for (auto& binding : dsl->bindings)
				{
					if (binding.type == DescriptorUniformBuffer || binding.type == DescriptorStorageBuffer)
						binding.ui = find_udt(sh(binding.name.c_str()), db);
				}
				dsls.push_back(dsl);
			}

			auto pc_ui = find_udt("PushConstant"_h, db);

			auto ret = PipelineLayout::create(dsls, pc_ui ? pc_ui->size : 0);
			ret->db = std::move(db);
			ret->pc_ui = pc_ui;
			return ret;
		}

		struct PipelineLayoutCreate : PipelineLayout::Create
		{
			PipelineLayoutPtr operator()(std::span<DescriptorSetLayoutPtr> dsls, uint push_constant_size) override
			{
				auto ret = new PipelineLayoutPrivate;
				ret->dsls.assign(dsls.begin(), dsls.end());
				ret->pc_sz = push_constant_size;

				std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts;
				vk_descriptor_set_layouts.resize(dsls.size());
				for (auto i = 0; i < dsls.size(); i++)
					vk_descriptor_set_layouts[i] = dsls[i]->vk_descriptor_set_layout;

				VkPushConstantRange vk_pushconstant_range;
				vk_pushconstant_range.offset = 0;
				vk_pushconstant_range.size = push_constant_size;
				vk_pushconstant_range.stageFlags = to_vk_flags<ShaderStageFlags>(ShaderStageAll);

				VkPipelineLayoutCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				info.flags = 0;
				info.pNext = nullptr;
				info.setLayoutCount = vk_descriptor_set_layouts.size();
				info.pSetLayouts = vk_descriptor_set_layouts.data();
				info.pushConstantRangeCount = push_constant_size > 0 ? 1 : 0;
				info.pPushConstantRanges = push_constant_size > 0 ? &vk_pushconstant_range : nullptr;

				check_vk_result(vkCreatePipelineLayout(device->vk_device, &info, nullptr, &ret->vk_pipeline_layout));
				register_object(ret->vk_pipeline_layout, "Pipeline Layout", ret);

				{
					D3D12_ROOT_SIGNATURE_DESC signature_desc;
					std::vector<D3D12_ROOT_PARAMETER> parameters;
					for (auto i = 0; i < dsls.size(); i++)
					{
						auto dsl = dsls[i];
						for (auto j = 0; j < dsl->bindings.size(); j++)
						{
							auto& binding = dsl->bindings[j];
							D3D12_ROOT_PARAMETER parameter;

							parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
						}
					}
					signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
					signature_desc.pParameters = parameters.data();
					signature_desc.NumParameters = parameters.size();
					signature_desc.pStaticSamplers = nullptr;
					signature_desc.NumStaticSamplers = 0;
					ID3DBlob* signature = nullptr;
					ID3DBlob* error = nullptr;
					D3D12SerializeRootSignature(&signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error); // wtf?
					device->d3d12_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&ret->d3d12_signature));
					if (signature)
						signature->Release();
					if (error)
						error->Release();
				}

				return ret;
			}

		}PipelineLayout_create;
		PipelineLayout::Create& PipelineLayout::create = PipelineLayout_create;

		struct PipelineLayoutGet : PipelineLayout::Get
		{
			PipelineLayoutPtr operator()(const std::filesystem::path& _filename) override
			{
				auto filename = Path::get(_filename);

				for (auto& p : loaded_pipelinelayouts)
				{
					if (p->filename == filename)
					{
						p->ref++;
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
				compile_shader(ShaderPll, filename, {}, res_path);

				auto ret = PipelineLayoutPrivate::load_from_res(res_path);
				if (ret)
				{
					ret->filename = filename;
					ret->ref = 1;
					loaded_pipelinelayouts.emplace_back(ret);
				}
				return ret;
			}
		}PipelineLayout_get;
		PipelineLayout::Get& PipelineLayout::get = PipelineLayout_get;

		struct PipelineLayoutRelease : PipelineLayout::Release
		{
			void operator()(PipelineLayoutPtr pll) override
			{
				if (pll->ref == 1)
				{
					std::erase_if(loaded_pipelinelayouts, [&](const auto& i) {
						return i.get() == pll;
					});
				}
				else
					pll->ref--;
			}
		}PipelineLayout_release;
		PipelineLayout::Release& PipelineLayout::release = PipelineLayout_release;

		ShaderPrivate::ShaderPrivate()
		{
			shaders.push_back(this);
		}

		ShaderPrivate::~ShaderPrivate()
		{
			if (app_exiting) return;

			std::erase_if(shaders, [&](const auto& i) {
				return i == this;
			});

			if (vk_module)
			{
				vkDestroyShaderModule(device->vk_device, vk_module, nullptr);
				unregister_object(vk_module);
			}
		}

		void ShaderPrivate::recreate()
		{
			if (!filename.empty() && get_file_length(filename) > 0)
			{
				auto res_path = filename;
				res_path += defines_to_hash_str(defines);
				res_path += L".res";
				if (compile_shader(type, filename, defines, res_path))
				{
					auto new_sd = ShaderPrivate::load_from_res(res_path);
					if (new_sd)
					{
						db = std::move(new_sd->db);
						in_ui = new_sd->in_ui;
						out_ui = new_sd->out_ui;

						vkDestroyShaderModule(device->vk_device, vk_module, nullptr);
						unregister_object(vk_module);
						vk_module = new_sd->vk_module;
						new_sd->vk_module = 0;
						delete new_sd;
					}
				}
			}
		}

		ShaderPtr ShaderPrivate::load_from_res(const std::filesystem::path& filename)
		{
			std::ifstream file(filename);
			if (!file.good())
				return nullptr;
			LineReader res(file);

			TypeInfoDataBase db;
			DataSoup data_soup;

			res.read_block("spv:");
			data_soup.load(res);
			res.read_block("typeinfo:");
			db.load_from_string(res.to_string());
			file.close();

			auto ret = new ShaderPrivate;
			ret->db = std::move(db);
			ret->in_ui = find_udt("Input"_h, ret->db);
			ret->out_ui = find_udt("Output"_h, ret->db);

			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = data_soup.soup.size();
			shader_info.pCode = (uint*)data_soup.soup.data();
			check_vk_result(vkCreateShaderModule(device->vk_device, &shader_info, nullptr, &ret->vk_module));
			register_object(ret->vk_module, "Shader", ret);

			{
				ret->d3d12_byte_code.assign((char*)data_soup.soup.data(), data_soup.soup.size());
			}

			return ret;
		}

		struct ShaderCreate : Shader::Create
		{
			ShaderPtr operator()(ShaderStageFlags type, const std::string& content, const std::vector<std::string>& defines, const std::filesystem::path& dst, const std::filesystem::path& src) override
			{
				auto fn = dst;
				if (fn.empty())
					fn = L"!temp.res";

				std::filesystem::path temp_path = L"temp";
				if (!src.empty())
					temp_path = src.parent_path() / temp_path;
				std::ofstream file(temp_path);
				file << content;
				file.close();
				if (!src.empty())
					std::filesystem::last_write_time(temp_path, std::filesystem::last_write_time(src));

				compile_shader(type, temp_path, defines, fn);
				std::filesystem::remove(temp_path);

				auto ret = ShaderPrivate::load_from_res(fn);
				if (!ret)
					return nullptr;
				if (fn.c_str()[0] == L'!')
					std::filesystem::remove(fn);
				else
				{
					auto str = fn.wstring();
					if (auto p = str.find('!'); p != std::wstring::npos)
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
			ShaderPtr operator()(ShaderStageFlags type, const std::filesystem::path& _filename, const std::vector<std::string>& defines) override
			{
				auto filename = Path::get(_filename);

				for (auto& s : loaded_shaders)
				{
					if (s->filename == filename && s->defines == defines)
					{
						s->ref++;
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

				auto res_path = filename;
				res_path += defines_to_hash_str(defines);
				res_path += L".res";
				compile_shader(type, filename, defines, res_path);

				auto ret = ShaderPrivate::load_from_res(res_path);
				if (ret)
				{
					ret->type = type;
					ret->filename = filename;
					ret->defines = defines;
					ret->ref = 1;
					loaded_shaders.emplace_back(ret);
				}
				return ret;
			}
		}Shader_get;
		Shader::Get& Shader::get = Shader_get;

		struct ShaderRelease : Shader::Release
		{
			void operator()(ShaderPtr sd) override
			{
				if (sd->ref == 1)
				{
					std::erase_if(loaded_shaders, [&](const auto& i) {
						return i.get() == sd;
					});
				}
				else
					sd->ref--;
			}
		}Shader_release;
		Shader::Release& Shader::release = Shader_release;

		struct LoadedVertexinput
		{
			std::filesystem::path filename;
			std::vector<std::string> defines;
			TypeInfoDataBase db;
			UdtInfo* ui;
		};
		static std::vector<std::unique_ptr<LoadedVertexinput>> loaded_vertex_inputs;
		UdtInfo* get_vertex_input_ui(const std::filesystem::path& _filename, const std::vector<std::string>& defines)
		{
			auto filename = Path::get(_filename);

			for (auto& vi : loaded_vertex_inputs)
			{
				if (vi->filename == filename && vi->defines == defines)
					return vi->ui;
			}

			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find vertex input: %s\n", _filename.c_str());
				return nullptr;
			}

			auto res_path = filename;
			res_path += defines_to_hash_str(defines);
			res_path += L".res";
			compile_shader(ShaderVi, filename, defines, res_path);

			if (std::ifstream file(res_path); file.good())
			{
				LineReader res(file);

				TypeInfoDataBase db;

				res.read_block("typeinfo:");
				db.load_from_string(res.to_string());
				file.close();

				auto vi = new LoadedVertexinput;
				vi->filename = filename;
				vi->defines = defines;
				vi->db = std::move(db);
				vi->ui = find_udt("Input"_h, vi->db);
				loaded_vertex_inputs.emplace_back(vi);
				return vi->ui;
			}

			return nullptr;
		}

		void load_pipeline(PipelineType pipeline_type, const std::filesystem::path& filename, const std::vector<std::string>& _defines, void** ret)
		{
			auto parent_path = filename.parent_path();

			std::ifstream file(filename);
			if (!file.good())
			{
				wprintf(L"cannot find pipeline: %s\n", filename.c_str());
				*ret = nullptr;
				return;
			}
			LineReader res(file);

			PipelineInfo info;

			res.read_block("");

			struct Source
			{
				std::string segment;
				std::filesystem::path path;
				std::vector<std::string> defines;
			};

			Source								layout_source;
			std::map<ShaderStageFlags, Source>	shader_sources;
			std::vector<std::string>			renderpass_defines;
			std::vector<std::string>			pipeline_defines;
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
							auto ret = std::string(scope_and_name[1]);
							if (dst_and_value.size() > 1)
							{
								ret += '=';
								ret += dst_and_value[1];
							}
							return ret;
						};
						if (s == "rp")
							renderpass_defines.push_back(form_define());
						else if (s == "vert")
							shader_sources[ShaderStageVert].defines.push_back(form_define());
						else if (s == "tesc")
							shader_sources[ShaderStageTesc].defines.push_back(form_define());
						else if (s == "tese")
							shader_sources[ShaderStageTese].defines.push_back(form_define());
						else if (s == "geom")
							shader_sources[ShaderStageGeom].defines.push_back(form_define());
						else if (s == "frag")
							shader_sources[ShaderStageFrag].defines.push_back(form_define());
						else if (s == "comp")
							shader_sources[ShaderStageComp].defines.push_back(form_define());
						else if (s == "task")
							shader_sources[ShaderStageTask].defines.push_back(form_define());
						else if (s == "mesh")
							shader_sources[ShaderStageMesh].defines.push_back(form_define());
						else if (s == "all_shader")
						{
							for (auto i = 0; ; i++)
							{
								auto stage = (ShaderStageFlags)(1 << i);
								if (ShaderStageAll & stage)
									shader_sources[stage].defines.push_back(form_define());
								else
									break;
							}
						}
					}
				}
			}

			UnserializeTextSpec spec;
			spec.typed_delegates[TypeInfo::get<PipelineLayout*>()] = [&](const TextSerializeNode& src)->void* {
				auto value = src.value();
				if (value.starts_with("0x"))
				{
					auto layout = (PipelineLayoutPtr)s2u_hex<uint64>(value.substr(2));
					layout_source.path = layout->filename;
					return layout;
				}
				if (value.starts_with("@"))
				{
					layout_source.segment = value;
					return INVALID_POINTER;
				}
				auto layout = PipelineLayout::get(Path::combine(parent_path, src.value()));
				layout_source.path = layout->filename;
				return layout;
			};
			spec.typed_delegates[TypeInfo::get<Shader*>()] = [&](const TextSerializeNode& src)->void* {
				auto value = src.value();
				if (!value.empty())
				{
					if (value[0] == '@')
					{
						ShaderStageFlags type = ShaderStageNone;
						if		(value.starts_with("@vert"))
							type = ShaderStageVert;
						else if (value.starts_with("@tesc"))
							type = ShaderStageTesc;
						else if (value.starts_with("@tese"))
							type = ShaderStageTese;
						else if (value.starts_with("@geom"))
							type = ShaderStageGeom;
						else if (value.starts_with("@frag"))
							type = ShaderStageFrag;
						else if (value.starts_with("@comp"))
							type = ShaderStageComp;
						else if (value.starts_with("@task"))
							type = ShaderStageTask;
						else if (value.starts_with("@mesh"))
							type = ShaderStageMesh;
						if (type != ShaderStageNone)
							shader_sources[type].segment = value;
						return INVALID_POINTER;
					}
					auto fn = Path::combine(parent_path, value);
					auto stage = stage_from_ext(fn);
					if (stage != ShaderStageNone)
					{
						shader_sources[stage].path = fn;
						return INVALID_POINTER;
					}
				}
				else
				{
					auto fn = Path::combine(parent_path, src.value("filename"));
					auto stage = stage_from_ext(fn);
					if (stage != ShaderStageNone)
					{
						auto defines = format_defines(src.value("defines"));
						shader_sources[stage].defines.insert(shader_sources[stage].defines.end(), defines.begin(), defines.end());
						shader_sources[stage].path = fn;
						return INVALID_POINTER;
					}
				}
				return INVALID_POINTER;
			};
			spec.typed_delegates[TypeInfo::get<Renderpass*>()] = [&](const TextSerializeNode& src)->void* {
				auto value = src.value();
				if (value.starts_with("0x"))
					return (void*)s2u_hex<uint64>(value.substr(2));
				if (!value.empty())
				{
					auto defines = renderpass_defines;
					std::sort(defines.begin(), defines.end());
					return Renderpass::get(Path::combine(parent_path, value), defines);
				}
				else
				{
					auto defines = format_defines(src.value("defines"));
					defines.insert(defines.end(), renderpass_defines.begin(), renderpass_defines.end());
					std::sort(defines.begin(), defines.end());
					return Renderpass::get(Path::combine(parent_path, src.value("filename")), defines);
				}
				return INVALID_POINTER;
			};
			std::sort(pipeline_defines.begin(), pipeline_defines.end());
			std::vector<std::pair<std::string, std::string>> splited_pipeline_defines;
			for (auto& d : pipeline_defines)
			{
				auto sp = SUS::split(d, '=');
				if (sp.size() == 2)
					splited_pipeline_defines.emplace_back(sp[0], sp[1]);
			}
			unserialize_text(res, &info, spec, splited_pipeline_defines);

			if (!layout_source.segment.empty())
			{
				res.read_block(layout_source.segment, "@");
				layout_source.segment = res.to_string();
			}
			for (auto& s : shader_sources)
			{
				if (!s.second.segment.empty())
				{
					res.read_block(s.second.segment, "@");
					s.second.segment = res.to_string();
				}
			}

			file.close();

			static int create_id = 0;
			if (filename.empty())
				create_id++;

			if (!layout_source.segment.empty())
			{
				auto path = !filename.empty() ? filename.wstring() + L"!pll" : L"!" + wstr(create_id);
				layout_source.path = path;
				if (!std::filesystem::exists(layout_source.path) || std::filesystem::last_write_time(layout_source.path) < std::filesystem::last_write_time(filename))
				{
					std::ofstream file(layout_source.path);
					file << layout_source.segment;
					file.close();
				}
				layout_source.path = std::filesystem::canonical(layout_source.path);
				info.layout = PipelineLayout::get(layout_source.path);
			}
			for (auto& s : shader_sources)
			{
				if (!s.second.segment.empty())
				{
					if (!layout_source.path.empty())
						s.second.segment = "#include \"" + layout_source.path.string() + "\"\n\n" + s.second.segment;
					std::sort(s.second.defines.begin(), s.second.defines.end());
					info.shaders.push_back(Shader::create(s.first, s.second.segment, s.second.defines,
						!filename.empty() ? filename.wstring() + (L"!" + get_stage_str(s.first) + defines_to_hash_str(s.second.defines) + L".res") : L"!" + wstr(create_id) + L".res", filename));
				}
				else if (!s.second.path.empty())
				{
					// load shader here so we can include the pll
					if (!layout_source.path.empty())
						s.second.defines.push_back("__add_line__=#include \"" + layout_source.path.string() + "\"");
					std::sort(s.second.defines.begin(), s.second.defines.end());
					info.shaders.push_back(Shader::get(s.first, s.second.path, s.second.defines));
				}
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
								std::string meta;
								vi.metas.get("Location"_h, &meta);
								va.location = s2t<uint>(meta);
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
								else if (vi.type == TypeInfo::get<int>() || vi.type == TypeInfo::get<uint>())
									va.format = Format_R32_UINT;
								else if (vi.type == TypeInfo::get<ivec4>() || vi.type == TypeInfo::get<uvec4>())
									va.format = Format_R32G32B32A32_INT;
							}
						}
						break;
					}
				}
			}

			if (pipeline_type == PipelineGraphics)
			{
				for (auto& att : info.renderpass->attachments)
				{
					if (att.sample_count != SampleCount_1)
					{
						info.sample_count = att.sample_count;
						break;
					}
				}
				*ret = GraphicsPipeline::create(info);
				return;
			}
			else if (!info.shaders.empty())
			{
				*ret = ComputePipeline::create(info);
				return;
			}
			*ret = nullptr;
		}

		GraphicsPipelinePrivate::GraphicsPipelinePrivate()
		{
			graphics_pipelines.push_back(this);
		}

		GraphicsPipelinePrivate::~GraphicsPipelinePrivate()
		{
			if (app_exiting) return;

			std::erase_if(graphics_pipelines, [&](const auto& i) {
				return i == this;
			});

			if (layout->ref == 0)
				delete layout;
			for (auto sd : shaders)
			{
				if (sd->ref == 0)
					delete sd;
			}

			if (vk_pipeline)
			{
				vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
				unregister_object(vk_pipeline);
			}
			for (auto& v : renderpass_variants)
			{
				vkDestroyPipeline(device->vk_device, v.second, nullptr);
				unregister_object(v.second);
			}

			if (d3d12_pipeline)
				d3d12_pipeline->Release();
		}

		VkPipeline GraphicsPipelinePrivate::get_dynamic_pipeline(RenderpassPtr rp, uint sp)
		{
			if (vk_pipeline && rp == renderpass)
				return vk_pipeline;
			auto it = renderpass_variants.find(rp);
			if (it != renderpass_variants.end())
				return it->second;
			PipelineInfo info = *this;
			info.renderpass = rp;
			info.subpass_index = sp;
			for (auto& att : info.renderpass->attachments)
			{
				if (att.sample_count != SampleCount_1)
				{
					info.sample_count = att.sample_count;
					break;
				}
			}
			auto new_pl = create(info);
			if (!new_pl)
				return nullptr;
			auto ret = new_pl->vk_pipeline;
			new_pl->vk_pipeline = nullptr;
			renderpass_variants.emplace(rp, ret);
			return ret;
		}

		void GraphicsPipelinePrivate::recreate()
		{
			PipelineInfo info = *this;
			GraphicsPipelinePtr new_pl = nullptr;
			if (!filename.empty() && get_file_length(filename) > 0)
			{
				load_pipeline(PipelineGraphics, filename, defines, (void**)&new_pl);
				if (!new_pl)
					return;
			}
			else
			{
				auto new_pl = create(info);
				if (!new_pl)
					return;
			}

			vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
			unregister_object(vk_pipeline);
			vk_pipeline = new_pl->vk_pipeline;
			new_pl->vk_pipeline = 0;
			delete new_pl;

			for (auto& v : renderpass_variants)
			{
				vkDestroyPipeline(device->vk_device, v.second, nullptr);
				unregister_object(v.second);
			}
			renderpass_variants.clear();
		}

		struct GraphicsPipelineCreate : GraphicsPipeline::Create
		{
			GraphicsPipelinePtr operator()(const PipelineInfo& info) override
			{
				auto ret = new GraphicsPipelinePrivate;
				*(PipelineInfo*)ret = info;

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
					dst.stage = to_vk(shader->type);
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
						dst_att.format = to_vk(src_att.format);
						vk_vi_attributes.push_back(dst_att);
					}
					dst_buf.inputRate = to_vk(src_buf.rate);
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
				assembly_state.topology = to_vk(info.primitive_topology);
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
				raster_state.depthClampEnable = info.depth_clamp;
				raster_state.rasterizerDiscardEnable = info.rasterizer_discard;
				raster_state.polygonMode = to_vk(info.polygon_mode);
				raster_state.cullMode = to_vk(info.cull_mode);
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
					auto& res_atts = info.renderpass->subpasses[info.subpass_index].color_resolve_attachments;
					multisample_state.rasterizationSamples = to_vk(!res_atts.empty() ? info.renderpass->attachments[res_atts[0]].sample_count : SampleCount_1);
				}
				else
					multisample_state.rasterizationSamples = to_vk(info.sample_count);
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
				depth_stencil_state.depthCompareOp = to_vk(info.depth_compare_op);
				depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
				depth_stencil_state.minDepthBounds = 0;
				depth_stencil_state.maxDepthBounds = 0;
				depth_stencil_state.stencilTestEnable = info.stencil_test;
				depth_stencil_state.front.compareOp = to_vk(info.stencil_compare_op);
				depth_stencil_state.front.passOp = to_vk(info.stencil_op);
				depth_stencil_state.front.failOp = to_vk(info.stencil_op);
				depth_stencil_state.front.depthFailOp = to_vk(info.stencil_op);
				depth_stencil_state.front.compareMask = 0xff;
				depth_stencil_state.front.writeMask = 0xff;
				depth_stencil_state.front.reference = 1;
				depth_stencil_state.back = depth_stencil_state.front;

				vk_blend_attachment_states.resize(info.renderpass->subpasses[info.subpass_index].color_attachments.size());
				for (auto& a : vk_blend_attachment_states)
				{
					a.blendEnable = VK_FALSE;
					a.srcColorBlendFactor = to_vk(BlendFactorZero);
					a.dstColorBlendFactor = to_vk(BlendFactorZero);
					a.colorBlendOp = to_vk(BlendOpAdd);
					a.srcAlphaBlendFactor = to_vk(BlendFactorZero);
					a.dstAlphaBlendFactor = to_vk(BlendFactorZero);
					a.alphaBlendOp = to_vk(BlendOpAdd);
					a.colorWriteMask = to_vk_flags<VkColorComponentFlags>(ColorComponentAll);
				}
				if (vk_blend_attachment_states.size() >= info.blend_options.size())
				{
					for (auto i = 0; i < vk_blend_attachment_states.size(); i++)
					{
						auto& src = i < info.blend_options.size() ? info.blend_options[i] : BlendOption();
						auto& dst = vk_blend_attachment_states[i];
						dst.blendEnable = src.enable;
						dst.srcColorBlendFactor = to_vk(src.src_color);
						dst.dstColorBlendFactor = to_vk(src.dst_color);
						dst.colorBlendOp = to_vk(src.color_op);
						dst.srcAlphaBlendFactor = to_vk(src.src_alpha);
						dst.dstAlphaBlendFactor = to_vk(src.dst_alpha);
						dst.alphaBlendOp = to_vk(src.alpha_op);
						dst.colorWriteMask = to_vk_flags<VkColorComponentFlags>(src.color_write_mask);
					}
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
					vk_dynamic_states.push_back(to_vk((DynamicState)info.dynamic_states[i]));
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

				check_vk_result(vkCreateGraphicsPipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &ret->vk_pipeline));
				register_object(ret->vk_pipeline, "Pipeline", ret);

				{
					D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
					std::vector<D3D12_INPUT_ELEMENT_DESC> dx_input_elements;
					for (auto i = 0; i < info.vertex_buffers.size(); i++)
					{
						auto& buf = info.vertex_buffers[i];
						auto offset = 0;
						for (auto j = 0; j < buf.attributes.size(); j++)
						{
							auto& src_att = buf.attributes[j];
							D3D12_INPUT_ELEMENT_DESC element;
							element.SemanticName = "ATTRIBUTE";
							element.SemanticIndex = src_att.location;
							element.Format = to_dx(src_att.format);
							element.InputSlot = i;
							if (src_att.offset != -1)
								offset = src_att.offset;
							element.AlignedByteOffset = offset;
							offset += format_size(src_att.format);
							element.InputSlotClass = buf.rate == VertexInputRateVertex ?
								D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
							element.InstanceDataStepRate = 0; // TOOD: use the stride of input buffer
							dx_input_elements.push_back(element);
						}
					}
					desc.InputLayout.pInputElementDescs = dx_input_elements.data();
					desc.InputLayout.NumElements = dx_input_elements.size();
					desc.pRootSignature = info.layout->d3d12_signature;
					for (auto i = 0; i < info.shaders.size(); i++)
					{
						auto& shader = info.shaders[i];
						switch (shader->type)
						{
						case ShaderStageVert:
							desc.VS.pShaderBytecode = shader->d3d12_byte_code.data();
							desc.VS.BytecodeLength = shader->d3d12_byte_code.size();
							break;
						case ShaderStageFrag:
							desc.PS.pShaderBytecode = shader->d3d12_byte_code.data();
							desc.PS.BytecodeLength = shader->d3d12_byte_code.size();
							break;
						}
					}
					desc.RasterizerState.FillMode = to_dx(info.polygon_mode);
					desc.RasterizerState.CullMode = to_dx(info.cull_mode);
					desc.RasterizerState.FrontCounterClockwise = true;
					desc.RasterizerState.DepthBias = 0;
					desc.RasterizerState.DepthBiasClamp = 0.f;
					desc.RasterizerState.SlopeScaledDepthBias = 0.f;
					desc.RasterizerState.DepthClipEnable = false;
					desc.RasterizerState.MultisampleEnable = info.sample_count != SampleCount_1;
					desc.RasterizerState.AntialiasedLineEnable = false; // TODO: dx can do that?
					desc.RasterizerState.ForcedSampleCount = 0; // TODO: unknown..
					desc.DepthStencilState.DepthEnable = info.depth_test;
					desc.DepthStencilState.DepthWriteMask = info.depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
					desc.DepthStencilState.DepthFunc = to_dx(info.depth_compare_op);
					desc.DepthStencilState.StencilEnable = info.stencil_test;
					desc.DepthStencilState.StencilReadMask = 0xff;
					desc.DepthStencilState.StencilWriteMask = 0xff;
					desc.DepthStencilState.FrontFace.StencilFunc = to_dx(info.stencil_compare_op);
					desc.DepthStencilState.FrontFace.StencilFailOp = to_dx(info.stencil_op);
					desc.DepthStencilState.FrontFace.StencilDepthFailOp = to_dx(info.stencil_op);
					desc.DepthStencilState.FrontFace.StencilPassOp = to_dx(info.stencil_op);
					desc.DepthStencilState.BackFace = desc.DepthStencilState.FrontFace;
					desc.PrimitiveTopologyType = to_dx(info.primitive_topology);
					desc.SampleMask = 0xffffffff;
					desc.NumRenderTargets = 1; // TODO: fix
					desc.RTVFormats[0] = to_dx(info.renderpass->attachments[0].format); // TODO: fix
					desc.SampleDesc.Count = 1;
					check_dx_result(device->d3d12_device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&ret->d3d12_pipeline)));
				}

				return ret;
			}

			GraphicsPipelinePtr operator()(const std::string& content, const std::vector<std::string>& defines) override
			{
				std::filesystem::path fn = L"!temp.pipeline";

				std::ofstream file(fn);
				file << SUS::get_ltrimed(content);
				file.close();

				GraphicsPipelinePtr ret;
				load_pipeline(PipelineGraphics, fn, defines, (void**)&ret);
				std::filesystem::remove(fn);
				return ret;
			}
		}GraphicsPipeline_create;
		GraphicsPipeline::Create& GraphicsPipeline::create = GraphicsPipeline_create;

		struct GraphicsPipelineGet : GraphicsPipeline::Get
		{
			GraphicsPipelinePtr operator()(const std::filesystem::path& _filename, const std::vector<std::string>& defines) override
			{
				auto filename = Path::get(_filename);

				for (auto& pl : loaded_graphics_pipelines)
				{
					if (pl->filename == filename && pl->defines == defines)
					{
						pl->ref++;
						return pl.get();
					}
				}

				GraphicsPipelinePtr ret;
				load_pipeline(PipelineGraphics, filename, defines, (void**)&ret);
				if (ret)
				{
					ret->filename = filename;
					ret->defines = defines;
					ret->ref = 1;
					loaded_graphics_pipelines.emplace_back(ret);
				}
				return ret;
			}
		}GraphicsPipeline_get;
		GraphicsPipeline::Get& GraphicsPipeline::get = GraphicsPipeline_get;

		struct GraphicsPipelineRelease : GraphicsPipeline::Release
		{
			void operator()(GraphicsPipelinePtr pl) override
			{
				if (pl->ref == 1)
				{
					std::erase_if(loaded_graphics_pipelines, [&](const auto& i) {
						return i.get() == pl;
					});
				}
				else
					pl->ref--;
			}
		}GraphicsPipeline_release;
		GraphicsPipeline::Release& GraphicsPipeline::release = GraphicsPipeline_release;

		ComputePipelinePrivate::~ComputePipelinePrivate()
		{
			if (app_exiting) return;

			vkDestroyPipeline(device->vk_device, vk_pipeline, nullptr);
			unregister_object(vk_pipeline);
		}

		struct ComputePipelineCreate : ComputePipeline::Create
		{
			ComputePipelinePtr operator()(const PipelineInfo& info) override
			{
				auto ret = new ComputePipelinePrivate;
				*(PipelineInfo*)ret = info;

				VkComputePipelineCreateInfo pipeline_info;
				pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
				pipeline_info.pNext = nullptr;
				pipeline_info.flags = 0;

				pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				pipeline_info.stage.flags = 0;
				pipeline_info.stage.pNext = nullptr;
				pipeline_info.stage.pSpecializationInfo = nullptr;
				pipeline_info.stage.pName = "main";
				pipeline_info.stage.stage = to_vk(ShaderStageComp);
				pipeline_info.stage.module = info.shaders[0]->vk_module;

				pipeline_info.basePipelineHandle = 0;
				pipeline_info.basePipelineIndex = 0;
				pipeline_info.layout = info.layout->vk_pipeline_layout;

				check_vk_result(vkCreateComputePipelines(device->vk_device, 0, 1, &pipeline_info, nullptr, &ret->vk_pipeline));
				register_object(ret->vk_pipeline, "Pipeline", ret);

				return ret;
			}

			ComputePipelinePtr operator()(const std::string& content, const std::vector<std::string>& defines, const std::string& key) override
			{
				return nullptr;
			}
		}ComputePipeline_create;
		ComputePipeline::Create& ComputePipeline::create = ComputePipeline_create;

		struct ComputePipelineGet : ComputePipeline::Get
		{
			ComputePipelinePtr operator()(const std::filesystem::path& _filename, const std::vector<std::string>& defines) override
			{
				auto filename = Path::get(_filename);

				for (auto& pl : loaded_compute_pipelines)
				{
					if (pl->filename == filename && pl->defines == defines)
					{
						pl->ref++;
						return pl.get();
					}
				}

				ComputePipelinePtr ret;
				load_pipeline(PipelineCompute, filename, defines, (void**)&ret);
				if (ret)
				{
					ret->filename = filename;
					ret->defines = defines;
					ret->ref = 1;
					loaded_compute_pipelines.emplace_back(ret);
				}
				return ret;
			}
		}ComputePipeline_get;
		ComputePipeline::Get& ComputePipeline::get = ComputePipeline_get;

		struct ComputePipelineRelease : ComputePipeline::Release
		{
			void operator()(ComputePipelinePtr pl) override
			{
				if (pl->ref == 1)
				{
					if (pl->layout->ref == 0)
						delete pl->layout;
					for (auto sd : shaders)
					{
						if (sd->ref == 0)
							delete sd;
					}
					std::erase_if(loaded_compute_pipelines, [&](const auto& i) {
						return i.get() == pl;
					});
				}
				else
					pl->ref--;
			}
		}ComputePipeline_release;
		ComputePipeline::Release& ComputePipeline::release = ComputePipeline_release;
	}
}
