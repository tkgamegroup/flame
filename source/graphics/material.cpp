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
		static std::vector<std::unique_ptr<MaterialPrivate>> materials;

		MaterialPrivate::MaterialPrivate()
		{
			// reference the Texture, or else the typeinfo will be lost
			textures.resize(8);
		}

		void MaterialPrivate::save(const std::filesystem::path& filename)
		{
			pugi::xml_document doc;
			auto doc_root = doc.append_child("material");

			serialize_xml(this, doc_root);
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

				for (auto& m : materials)
				{
					if (m->filename == filename)
						return m.get();
				}

				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("material"))
				{
					printf("material does not exist: %s\n", filename.string().c_str());
					return nullptr;
				}

				auto ret = new MaterialPrivate;
				unserialize_xml(doc_root, ret);

				ret->ref = 1;
				materials.emplace_back(ret);
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
					std::erase_if(materials, [&](const auto& i) {
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
