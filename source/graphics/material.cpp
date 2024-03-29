#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "../foundation/typeinfo_serialize.h"
#include "../foundation/blueprint.h"
#include "image_private.h"
#include "material_private.h"

namespace flame
{
	namespace graphics
	{
		std::vector<MaterialPtr> materials;
		std::vector<std::unique_ptr<MaterialT>> loaded_materials;

		MaterialPtr default_material = new MaterialPrivate;
		MaterialPtr default_red_material = new MaterialPrivate;
		MaterialPtr default_green_material = new MaterialPrivate;
		MaterialPtr default_blue_material = new MaterialPrivate;
		MaterialPtr default_yellow_material = new MaterialPrivate;
		MaterialPtr default_purple_material = new MaterialPrivate;
		MaterialPtr default_cyan_material = new MaterialPrivate;

		struct DefaultMaterialInitializer
		{
			DefaultMaterialInitializer()
			{
				default_red_material->color = vec4(1.f, 0.f, 0.f, 1.f);
				default_green_material->color = vec4(0.f, 1.f, 0.f, 1.f);
				default_blue_material->color = vec4(0.f, 0.f, 1.f, 1.f);
				default_yellow_material->color = vec4(1.f, 1.f, 0.f, 1.f);
				default_purple_material->color = vec4(1.f, 0.f, 1.f, 1.f);
				default_cyan_material->color = vec4(0.f, 1.f, 1.f, 1.f);
			}
		};
		static DefaultMaterialInitializer default_material_initializer;

		MaterialPrivate::MaterialPrivate()
		{
			materials.push_back(this);
		}

		MaterialPrivate::~MaterialPrivate()
		{
			if (app_exiting) return;

			std::erase_if(materials, [&](const auto& i) {
				return i == this;
			});
		}

		void MaterialPrivate::set_color(const vec4& v)
		{
			if (color == v)
				return;
			color = v;

			data_changed("color"_h);
		}

		void MaterialPrivate::set_metallic(float v)
		{
			if (metallic == v)
				return;
			metallic = v;

			data_changed("metallic"_h);
		}

		void MaterialPrivate::set_roughness(float v)
		{
			if (roughness == v)
				return;
			roughness = v;

			data_changed("roughness"_h);
		}

		void MaterialPrivate::set_emissive(const vec4& v)
		{
			if (emissive == v)
				return;
			emissive = v;

			data_changed("emissive"_h);
		}

		void MaterialPrivate::set_tiling(float v)
		{
			if (tiling == v)
				return;
			tiling = v;

			data_changed("tiling"_h);
		}

		void MaterialPrivate::set_render_queue(RenderQueue q)
		{
			if (render_queue == q)
				return;
			render_queue = q;

			data_changed("render_queue"_h);
		}

		void MaterialPrivate::set_mirror(bool v) 
		{
			if (mirror == v)
				return;
			mirror = v;

			data_changed("mirror"_h);
		}

		void MaterialPrivate::set_color_map(int i)
		{
			i = clamp(i, -1, (int)textures.size() - 1);
			if (color_map == i)
				return;
			color_map = i;

			data_changed("color_map"_h);
		}

		void MaterialPrivate::set_normal_map(int i)
		{
			i = clamp(i, -1, (int)textures.size() - 1);
			if (normal_map == i)
				return;
			normal_map = i;

			data_changed("normal_map"_h);
		}

		void MaterialPrivate::set_normal_map_strength(float v) 
		{
			if (normal_map_strength == v)
				return;
			normal_map_strength = v;

			data_changed("normal_map_strength"_h);
		}

		void MaterialPrivate::set_metallic_map(int i)
		{
			i = clamp(-1, (int)textures.size(), i);
			if (metallic_map == i)
				return;
			metallic_map = i;

			data_changed("metallic_map"_h);
		}

		void MaterialPrivate::set_roughness_map(int i)
		{
			i = clamp(-1, (int)textures.size(), i);
			if (roughness_map == i)
				return;
			roughness_map = i;

			data_changed("roughness_map"_h);
		}

		void MaterialPrivate::set_emissive_map(int i)
		{
			i = clamp(i, -1, (int)textures.size() - 1);
			if (emissive_map == i)
				return;
			emissive_map = i;

			data_changed("emissive_map"_h);
		}

		void MaterialPrivate::set_emissive_map_strength(float v) 
		{
			if (emissive_map_strength == v)
				return;
			emissive_map_strength = v;

			data_changed("emissive_map_strength"_h);
		}

		void MaterialPrivate::set_alpha_map(int i)
		{
			i = clamp(i, -1, (int)textures.size() - 1);
			if (alpha_map == i)
				return;
			alpha_map = i;

			data_changed("alpha_map"_h);
		}

		void MaterialPrivate::set_splash_map(int i)
		{
			i = clamp(i, -1, (int)textures.size() - 1);
			if (splash_map == i)
				return;
			splash_map = i;

			data_changed("splash_map"_h);
		}

		void MaterialPrivate::set_code_file(const std::filesystem::path& path)
		{
			if (code_file == path)
				return;
			code_file = path;

			generate_code();
			data_changed("code_file"_h);
		}

		void MaterialPrivate::set_defines(const std::vector<std::string>& _defines)
		{
			defines = _defines;

			data_changed("defines"_h);
		}

		void MaterialPrivate::set_textures(const std::vector<FileTexture>& _textures)
		{
			textures = _textures;

			data_changed("textures"_h);
		}

		void MaterialPrivate::generate_code(bool force)
		{
			if (code_file.empty())
				return;
			auto ext = code_file.extension();
			if (ext != L".bp")
				return;

			auto code_file_abs_path = Path::get(code_file);
			auto code_file_path = code_file_abs_path;
			code_file_path += L".glsl";
			if (!force && std::filesystem::exists(code_file_path) &&
				std::filesystem::last_write_time(code_file_path) > std::filesystem::last_write_time(code_file_abs_path))
				return;

			auto bp = Blueprint::get(code_file);
			auto bp_ins = BlueprintInstance::create(bp);
			std::ofstream file(code_file_path);

			std::string define_str;
			std::string function_str;

			if (auto it = bp_ins->groups.find("main"_h); it != bp_ins->groups.end())
			{
				auto& g = it->second;

				auto format_type_name = [](const std::string& name)->std::string {
					auto ret = name;
					SUS::strip_head_if(ret, "glm::");
					if (ret == "cvec4")
						ret = "vec4";
					return ret;
				};

				std::vector<std::pair<uint, BlueprintAttribute*>> output_slot_values;
				std::map<void*, uint> data_to_id;
				std::map<uint, std::string> id_to_var_name;
				for (auto& kv : g.slot_datas)
				{
					auto& arg = kv.second.attribute;
					if (!arg.type || arg.type->tag != TagD)
						continue;
					output_slot_values.emplace_back(kv.first, &arg); // we first put all slot datas into it, and then remove the input ones later
					data_to_id[arg.data] = kv.first;
				}

				std::function<void(BlueprintInstanceNode& n)> process_node;
				process_node = [&](BlueprintInstanceNode& n) {
					auto ori = n.original;
					if (ori)
					{
						auto get_input = [&](uint idx)->std::string {
							auto slot_id = ori->inputs[idx]->object_id;
							if (auto it = g.slot_datas.find(slot_id); it != g.slot_datas.end())
							{
								// remove the input ones from output_slot_datas
								std::erase_if(output_slot_values, [&](const auto& i) {
									return i.first == slot_id;
								});
								auto& arg = it->second.attribute;
								auto value_str = arg.type->serialize(arg.data);
								if (arg.type->tag == TagD)
								{
									auto ti = (TypeInfo_Data*)arg.type;
									if (ti->vec_size > 1)
										value_str = std::format("{}({})", format_type_name(ti->name), value_str);
								}
								return value_str;
							}
							if (auto it = data_to_id.find(n.inputs[idx].data); it != data_to_id.end())
							{
								if (auto it2 = id_to_var_name.find(it->second); it2 != id_to_var_name.end())
									return it2->second;
								return std::format("v_{}", it->second);
							}
							return "";
						};

						switch (ori->name_hash)
						{
						case "Scalar"_h:
							function_str += std::format("\tv_{} = {};\n",
								ori->outputs[0]->object_id,
								get_input(0));
							break;
						case "Vec2"_h:
							function_str += std::format("\tv_{} = vec2({}, {});\n",
								ori->outputs[0]->object_id,
								get_input(0),
								get_input(1));
							break;
						case "Vec3"_h:
							function_str += std::format("\tv_{} = vec3({}, {}, {});\n",
								ori->outputs[0]->object_id,
								get_input(0),
								get_input(1),
								get_input(2));
							break;
						case "Vec4"_h:
							function_str += std::format("\tv_{} = vec4({}, {}, {}, {});\n",
								ori->outputs[0]->object_id,
								get_input(0),
								get_input(1),
								get_input(2),
								get_input(3));
							break;
						case "Decompose"_h:
						{
							auto ti = (TypeInfo_Data*)n.inputs[0].type;
							switch (ti->vec_size)
							{
							case 2:
								function_str += std::format(
									"\tv_{} = {}.x;\n"
									"\tv_{} = {}.y;\n",
									ori->outputs[0]->object_id, get_input(0),
									ori->outputs[1]->object_id, get_input(0)
								);
								break;
							case 3:
								function_str += std::format(
									"\tv_{} = {}.x;\n"
									"\tv_{} = {}.y;\n"
									"\tv_{} = {}.z;\n",
									ori->outputs[0]->object_id, get_input(0),
									ori->outputs[1]->object_id, get_input(0),
									ori->outputs[2]->object_id, get_input(0)
								);
								break;
							case 4:
								function_str += std::format(
									"\tv_{} = {}.x;\n"
									"\tv_{} = {}.y;\n"
									"\tv_{} = {}.z;\n"
									"\tv_{} = {}.w;\n",
									ori->outputs[0]->object_id, get_input(0),
									ori->outputs[1]->object_id, get_input(0),
									ori->outputs[2]->object_id, get_input(0),
									ori->outputs[3]->object_id, get_input(0)
								);
								break;
							}
						}
							break;
						case "Add"_h:
							function_str += std::format("\tv_{} = {} + {};\n", ori->outputs[0]->object_id, get_input(0), get_input(1));
							break;
						case "Subtract"_h:
							function_str += std::format("\tv_{} = {} - {};\n", ori->outputs[0]->object_id, get_input(0), get_input(1));
							break;
						case "Multiply"_h:
							function_str += std::format("\tv_{} = {} * {};\n", ori->outputs[0]->object_id, get_input(0), get_input(1));
							break;
						case "Divide"_h:
							function_str += std::format("\tv_{} = {} / {};\n", ori->outputs[0]->object_id, get_input(0), get_input(1));
							break;
						case "Normalize"_h:
							function_str += std::format("\tv_{} = normalize({});\n", ori->outputs[0]->object_id, get_input(0));
							break;
						case "HSV Color"_h:
							function_str += std::format("\tv_{} = hsvColor({}, {}, {}, {});\n", ori->outputs[0]->object_id, get_input(0), get_input(1), get_input(2), get_input(3));
							break;
						case "Color To Vec4"_h:
							function_str += std::format("\tv_{} = {};\n", ori->outputs[0]->object_id, get_input(0));
							break;
						case "Perlin"_h:
							function_str += std::format("\tv_{} = perlin({});\n", ori->outputs[0]->object_id, get_input(0));
							break;
						case "Perlin Gradient"_h:
							function_str += std::format("\tv_{} = perlin_gradient({}, {});\n", ori->outputs[0]->object_id, get_input(0), get_input(1));
							break;
						case "Input"_h:
							if (auto idx = ori->find_output_i("i_color"_h); idx != -1)
							{
								auto slot_id = ori->outputs[idx]->object_id;
								std::erase_if(output_slot_values, [&](const auto& i) {
									return i.first == slot_id;
								});
								id_to_var_name[slot_id] = "color";
							}
							if (auto idx = ori->find_output_i("i_uv"_h); idx != -1)
							{
								auto slot_id = ori->outputs[idx]->object_id;
								std::erase_if(output_slot_values, [&](const auto& i) {
									return i.first == slot_id;
								});
								id_to_var_name[slot_id] = "i_uv";
							}
							if (auto idx = ori->find_output_i("i_coordw"_h); idx != -1)
							{
								auto slot_id = ori->outputs[idx]->object_id;
								std::erase_if(output_slot_values, [&](const auto& i) {
									return i.first == slot_id;
								});
								id_to_var_name[slot_id] = "i_coordw";
							}
							if (auto idx = ori->find_output_i("i_normal"_h); idx != -1)
							{
								auto slot_id = ori->outputs[idx]->object_id;
								std::erase_if(output_slot_values, [&](const auto& i) {
									return i.first == slot_id;
								});
								id_to_var_name[slot_id] = "i_normal";
							}
							break;
						case "Output"_h:
							function_str += "\tvec4 o_color;\n";
							function_str += "\tvec3 o_normal;\n";
							function_str += "\tfloat o_metallic;\n";
							function_str += "\tfloat o_roughness;\n";
							function_str += "\tvec3 o_emissive;\n";
							if (auto idx = ori->find_input_i("o_color"_h); idx != -1)
								function_str += std::format("\to_color = {};\n", get_input(idx));
							if (auto idx = ori->find_input_i("o_normal"_h); idx != -1)
								function_str += std::format("\to_normal = {};\n", get_input(idx));
							if (auto idx = ori->find_input_i("o_metallic"_h); idx != -1)
								function_str += std::format("\to_metallic = {};\n", get_input(idx));
							if (auto idx = ori->find_input_i("o_roughness"_h); idx != -1)
								function_str += std::format("\to_roughness = {};\n", get_input(idx));
							if (auto idx = ori->find_input_i("o_emissive"_h); idx != -1)
								function_str += std::format("\to_emissive = {};\n", get_input(idx));

							function_str += "\t#ifndef GBUFFER_PASS\n";
							function_str += "\tvec3 albedo = (1.0 - o_metallic) * o_color.rgb;\n";
							function_str += "\tvec3 f0 = mix(vec3(0.04), o_color.rgb, o_metallic);\n";
							function_str += "\to_color = vec4(shading(i_coordw, o_normal, o_metallic, albedo, f0, o_roughness, 1.0, o_emissive, false), o_color.a);\n";
							function_str += "\t#else\n";
							function_str += "\to_gbufferA = vec4(o_color.rgb, 0.0);\n";
							function_str += "\to_gbufferB = vec4(o_normal * 0.5 + 0.5, 0.0);\n";
							function_str += "\to_gbufferC = vec4(o_metallic, o_roughness, 0.0, 0.0);\n";
							function_str += "\to_gbufferD = vec4(o_emissive, 0.0);\n";
							function_str += "\t#endif\n";
							break;
						case "Block"_h:
							break;
						default:
							assert(0);
						}
					}
					for (auto& c : n.children)
						process_node(c);
				};
				process_node(g.root_node);

				file << "void material_main(MaterialInfo material, vec4 color)\n{\n";
				file << "#ifndef DEPTH_ONLY\n";
				for (auto& s : output_slot_values)
				{
					if (s.second->type && s.second->data)
						define_str += std::format("\t{} v_{};\n", format_type_name(s.second->type->name), s.first);
				}
				file << define_str;
				file << function_str;
				file << "#else\n";
				file << "\t#include <esm_value.glsl>\n";
				file << "#endif\n";
				file << "}\n";
			}

			file.close();
			delete bp_ins;
			Blueprint::release(bp);
		}

		void MaterialPrivate::save(const std::filesystem::path& filename)
		{
			auto base_path = Path::reverse(filename).parent_path();

			pugi::xml_document doc;
			auto doc_root = doc.append_child("material");

			SerializeXmlSpec spec;
			spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = [&](void* src) {
				auto& path = *(std::filesystem::path*)src;
				return Path::rebase(base_path, path).string();
			};

			for (auto it = defines.begin(); it != defines.end();)
			{
				if (it->empty() || (it->starts_with("frag:") && it->contains("_MAP=")))
					it = defines.erase(it);
				else
					it++;
			}

			serialize_xml(this, doc_root, spec);

			doc.save_file(filename.c_str());
		}

		struct MaterialCreate : Material::Create
		{
			MaterialPtr operator()() override
			{
				return new MaterialPrivate();
			}
		}Material_create;
		Material::Create& Material::create = Material_create;

		static void generate_shader_code_by_blueprint(Material* mat, BlueprintInstance* bp_ins, std::ofstream& file)
		{
		}

		struct MaterialGet : Material::Get
		{
			MaterialPtr operator()(const std::filesystem::path& _filename) override
			{
				if (_filename == L"default")
					return default_material;
				if (_filename == L"default_red")
					return default_red_material;
				if (_filename == L"default_green")
					return default_green_material;
				if (_filename == L"default_blue")
					return default_blue_material;
				if (_filename == L"default_yellow")
					return default_yellow_material;
				if (_filename == L"default_purple")
					return default_purple_material;
				if (_filename == L"default_cyan")
					return default_cyan_material;

				auto filename = Path::get(_filename);

				for (auto& m : loaded_materials)
				{
					if (m->filename == filename)
					{
						m->ref++;
						return m.get();
					}
				}

				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("material"))
				{
					printf("material does not exist or wrong format: %s\n", _filename.string().c_str());
					return nullptr;
				}

				auto ret = new MaterialPrivate;

				auto base_path = Path::reverse(filename).parent_path();
				UnserializeXmlSpec spec;
				spec.typed_delegates[TypeInfo::get<std::filesystem::path>()] = [&](const std::string& str, void* dst) {
					*(std::filesystem::path*)dst = Path::combine(base_path, str);
				};
				spec.general_delegate = [](const std::string& name, uint name_hash, TypeInfo* type, const std::string& src, void* dst) {
					if (name.ends_with("_map"))
					{
						*(int*)dst = s2t<int>(src);
						return true;
					}
					return false;
				};
				unserialize_xml(doc_root, ret, spec);

				ret->generate_code();

				ret->filename = filename;
				ret->ref = 1;
				loaded_materials.emplace_back(ret);
				return ret;
			}
		}Material_get;
		Material::Get& Material::get = Material_get;

		struct MaterialRelease : Material::Release
		{
			void operator()(MaterialPtr material) override
			{
				if (material == default_material)
					return;
				if (material->ref == 1)
				{
					std::erase_if(loaded_materials, [&](const auto& i) {
						return i.get() == material;
					});
				}
				else
					material->ref--;
			}
		}Material_release;
		Material::Release& Material::release = Material_release;
	}
}
