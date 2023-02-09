#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "../foundation/typeinfo_serialize.h"
#include "image_private.h"
#include "material_private.h"

namespace flame
{
	namespace graphics
	{
		MaterialPtr default_material = new MaterialPrivate;
		std::vector<MaterialPtr> materials;
		std::vector<std::unique_ptr<MaterialT>> loaded_materials;

		MaterialPrivate::MaterialPrivate()
		{
			textures.resize(8);

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

		void MaterialPrivate::set_opaque(bool v)
		{
			if (opaque == v)
				return;
			opaque = v;

			data_changed("opaque"_h);
		}

		void MaterialPrivate::set_sort(bool v) 
		{
			if (sort == v)
				return;
			sort = v;

			data_changed("sort"_h);
		}

		void MaterialPrivate::set_receive_ssr(bool v) 
		{
			if (receive_ssr == v)
				return;
			receive_ssr = v;

			data_changed("receive_ssr"_h);
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

		void MaterialPrivate::set_alpha_test(float v) 
		{
			if (alpha_test == v)
				return;
			alpha_test = v;

			data_changed("alpha_test"_h);
		}

		void MaterialPrivate::set_textures(const std::vector<Texture>& _textures)
		{
			textures = _textures;

			data_changed("textures"_h);
		}

		void MaterialPrivate::save(const std::filesystem::path& filename)
		{
			auto base_path = Path::reverse(filename).parent_path();

			pugi::xml_document doc;
			auto doc_root = doc.append_child("material");

			SerializeXmlSpec spec;
			spec.data_delegates[TypeInfo::get<std::filesystem::path>()] = [&](void* src) {
				auto& path = *(std::filesystem::path*)src;
				return Path::rebase(base_path, path).string();
			};

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

		struct MaterialGet : Material::Get
		{
			MaterialPtr operator()(const std::filesystem::path& _filename) override
			{
				if (_filename == L"default")
					return default_material;

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
					printf("material does not exist: %s\n", filename.string().c_str());
					return nullptr;
				}

				auto ret = new MaterialPrivate;

				auto base_path = Path::reverse(filename).parent_path();
				UnserializeXmlSpec spec;
				spec.data_delegates[TypeInfo::get<std::filesystem::path>()] = [&](const std::string& str, void* dst) {
					*(std::filesystem::path*)dst = Path::combine(base_path, str);
				};
				unserialize_xml(doc_root, ret, spec);

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
