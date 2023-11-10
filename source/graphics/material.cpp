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

		void MaterialPrivate::generate_code()
		{
			if (!code_file.empty())
			{
				auto ext = code_file.extension();
				if (ext == L".bp")
				{
					auto code_file_abs_path = Path::get(code_file);
					auto code_file_path = code_file_abs_path;
					code_file_path += L".glsl";
					if (!std::filesystem::exists(code_file_path) ||
						std::filesystem::last_write_time(code_file_path) < std::filesystem::last_write_time(code_file_abs_path))
					{
						auto bp = Blueprint::get(code_file);
						auto bp_ins = BlueprintInstance::create(bp);
						std::ofstream file(code_file_path);
						
						if (auto it = bp_ins->groups.find("main"_h); it != bp_ins->groups.end())
						{
							auto format_name = [](const std::string& name)->std::string {
								auto ret = name;
								SUS::strip_head_if(ret, "glm::");
								return ret;
							};

							auto& g = it->second;
							if (render_queue == RenderQueue::Opaque || render_queue == RenderQueue::AlphaTest)
							{

							}
							else if (render_queue == RenderQueue::Transparent)
							{
								for (auto& kv : g.slot_datas)
								{
									auto& arg = kv.second.attribute;
									file << std::format("{} v_{};\n", format_name(arg.type->name), kv.first);
								}

								file << "void material_main(MaterialInfo material, vec4 color)\n{\n";

								std::function<void(BlueprintInstanceNode& n)> process_node;
								process_node = [&](BlueprintInstanceNode& n) {
									auto ori = n.original;
									if (ori)
									{
										auto get_input_data_id = [&](uint idx)->int {
											auto slot_id = ori->inputs[idx]->object_id;
											if (auto it = g.slot_datas.find(slot_id); it != g.slot_datas.end())
												return slot_id;
											auto slot_data = n.inputs[idx].data;
											for (auto& kv : g.slot_datas)
											{
												if (kv.second.attribute.data == slot_data)
													return kv.first;
											}
											return -1;
										};

										for (auto i = 0; i < ori->inputs.size(); i++)
										{
											auto& input = ori->inputs[i];
											auto slot_id = input->object_id;
											if (auto it = g.slot_datas.find(slot_id); it != g.slot_datas.end())
											{
												auto& arg = it->second.attribute;
												file << std::format("\tv_{} = {};\n", it->first, arg.type->serialize(arg.data));
											}
										}

										switch (ori->name_hash)
										{
										case "Vec4"_h:
											file << std::format("\tv_{} = vec4(v_{}, v_{}, v_{}, v_{});\n", 
												ori->outputs[0]->object_id,
												get_input_data_id(0),
												get_input_data_id(1),
												get_input_data_id(2),
												get_input_data_id(3));
											break;
										case "Output"_h:
											if (auto o_color_idx = ori->find_input_i("o_color"_h); o_color_idx != -1)
											{
												file << std::format("\to_color = v_{};\n",
													get_input_data_id(o_color_idx));
											}
											break;
										}
									}
									for (auto& c : n.children)
										process_node(c);
								};
								process_node(g.root_node);

								file << "}\n";
							}
						}

						file.close();
						delete bp_ins;
						Blueprint::release(bp);
					}
				}
			}
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
