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

		void MaterialPrivate::save(const std::filesystem::path& filename)
		{
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

				UnserializeXmlSpec spec;
				unserialize_xml(doc_root, ret, spec);

				//ret->filename = filename;
				//if (auto n = doc_root.attribute("color"); n)
				//	ret->color = s2t<4, float>(std::string(n.value()));
				//if (auto n = doc_root.attribute("metallic"); n)
				//	ret->metallic = n.as_float();
				//if (auto n = doc_root.attribute("roughness"); n)
				//	ret->roughness = n.as_float();
				//if (auto n = doc_root.attribute("opaque"); n)
				//	ret->opaque = n.as_bool();
				//if (auto n = doc_root.attribute("sort"); n)
				//	ret->sort = n.as_bool();
				//if (auto n = doc_root.attribute("shader_file"); n)
				//	ret->shader_file = n.value();
				//if (auto n = n_material.attribute("shader_defines"); n)
				//	ret->shader_defines = n.value();

				//auto i = 0;
				//for (auto n_texture : doc_root)
				//{
				//	auto& dst = ret->textures[i];
				//	if (auto n = n_texture.attribute("filename"); n)
				//		dst.filename = n.value();
				//	if (auto n = n_texture.attribute("srgb"); n)
				//		dst.srgb = n.as_bool();
				//	if (auto n = n_texture.attribute("mag_filter"); n)
				//		TypeInfo::unserialize_t(n.value(), &dst.mag_filter);
				//	if (auto n = n_texture.attribute("min_filter"); n)
				//		TypeInfo::unserialize_t(n.value(), &dst.min_filter);
				//	if (auto n = n_texture.attribute("linear_mipmap"); n)
				//		dst.linear_mipmap = n.as_bool();
				//	if (auto n = n_texture.attribute("address_mode"); n)
				//		TypeInfo::unserialize_t(n.value(), &dst.address_mode);
				//	if (auto n = n_texture.attribute("auto_mipmap"); n)
				//		dst.auto_mipmap = n.as_bool();
				//	i++;
				//}

				materials.emplace_back(ret);
				return ret;
			}
		}Material_get;
		Material::Get& Material::get = Material_get;
	}
}
