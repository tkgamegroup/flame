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

		void MaterialPrivate::save(const std::filesystem::path& filename)
		{
			auto base_path = Path::reverse(filename).parent_path();

			pugi::xml_document doc;
			auto doc_root = doc.append_child("material");

			SerializeXmlSpec spec;
			spec.excludes.emplace_back(th<graphics::Material>(), "filename"_h);
			spec.excludes.emplace_back(th<graphics::Material>(), "ref"_h);
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
