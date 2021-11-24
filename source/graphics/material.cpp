#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "image_private.h"
#include "material_private.h"

namespace flame
{
	namespace graphics
	{
		MaterialPtr default_material = new MaterialPrivate;
		static std::vector<std::unique_ptr<MaterialPrivate>> materials;

		struct MaterialGet : Material::Get
		{
			MaterialPtr operator()(const std::filesystem::path& filename) override
			{
				for (auto& m : materials)
				{
					if (m->filename == filename)
						return m.get();
				}

				pugi::xml_document doc;
				pugi::xml_node n_material;
				if (!doc.load_file(filename.c_str()) || (n_material = doc.first_child()).name() != std::string("material"))
				{
					printf("material does not exist: %s\n", filename.string().c_str());
					return nullptr;
				}

				auto ret = new MaterialPrivate;
				ret->filename = filename;
				if (auto n = n_material.attribute("color"); n)
					ret->color = sto<4, float>(std::string(n.value()));
				if (auto n = n_material.attribute("metallic"); n)
					ret->metallic = n.as_float();
				if (auto n = n_material.attribute("roughness"); n)
					ret->roughness = n.as_float();
				if (auto n = n_material.attribute("opaque"); n)
					ret->opaque = n.as_bool();
				if (auto n = n_material.attribute("sort"); n)
					ret->sort = n.as_bool();
				if (auto n = n_material.attribute("pipeline_file"); n)
					ret->pipeline_file = n.value();
				if (auto n = n_material.attribute("pipeline_defines"); n)
					ret->pipeline_defines = n.value();

				auto i = 0;
				for (auto n_texture : n_material)
				{
					auto& dst = ret->textures[i];
					if (auto n = n_texture.attribute("filename"); n)
						dst.filename = n.value();
					if (auto n = n_texture.attribute("srgb"); n)
						dst.srgb = n.as_bool();
					if (auto n = n_texture.attribute("mag_filter"); n)
						TypeInfo::unserialize_es(n.value(), &dst.mag_filter);
					if (auto n = n_texture.attribute("min_filter"); n)
						TypeInfo::unserialize_es(n.value(), &dst.min_filter);
					if (auto n = n_texture.attribute("linear_mipmap"); n)
						dst.linear_mipmap = n.as_bool();
					if (auto n = n_texture.attribute("address_mode"); n)
						TypeInfo::unserialize_es(n.value(), &dst.address_mode);
					if (auto n = n_texture.attribute("auto_mipmap"); n)
						dst.auto_mipmap = n.as_bool();
					i++;
				}

				materials.emplace_back(ret);
				return ret;
			}
		}Material_get;
		Material::Get& Material::get = Material_get;
	}
}
