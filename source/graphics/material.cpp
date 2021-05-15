#include "../foundation/typeinfo.h"
#include "image_private.h"
#include "material_private.h"

#include <pugixml.hpp>

namespace flame
{
	namespace graphics
	{
		MaterialPrivate* default_material = new MaterialPrivate;
		std::vector<std::unique_ptr<MaterialPrivate>> loaded_materials;

		void MaterialPrivate::get_texture_file(uint idx, wchar_t* dst) const
		{
			if (idx < 4)
			{
				auto& src = textures[idx];
				if (src.filename.empty())
					dst[0] = 0;
				else
				{
					auto path = filename.parent_path() / src.filename;
					if (!std::filesystem::exists(path))
					{
						path = pipeline_file;
						get_engine_path(path, L"assets\\shaders");
					}
					wcscpy(dst, path.c_str());
				}
			}
		}

		SamplerPtr MaterialPrivate::get_texture_sampler(DevicePtr device, uint idx) const
		{
			if (idx < 4)
			{
				auto& src = textures[idx];
				return SamplerPrivate::get(device, src.mag_filter, src.min_filter, src.linear_mipmap, src.address_mode);
			}
			return nullptr;
		}

		void MaterialPrivate::get_pipeline_file(wchar_t* dst) const
		{
			auto path = filename.parent_path() / pipeline_file;
			if (!std::filesystem::exists(path))
			{
				path = pipeline_file;
				get_engine_path(path, L"assets\\shaders");
			}
			wcscpy(dst, path.c_str());
		}

		MaterialPrivate* MaterialPrivate::get(const std::filesystem::path& filename)
		{
			for (auto& m : loaded_materials)
			{
				if (m->filename == filename)
					return m.get();
			}

			pugi::xml_document doc;
			pugi::xml_node n_material;
			if (!doc.load_file(filename.c_str()) || (n_material = doc.first_child()).name() != std::string("material"))
			{
				printf("model does not exist: %s\n", filename.string().c_str());
				return nullptr;
			}

			auto ret = new MaterialPrivate;
			ret->filename = filename;
			if (auto n = n_material.attribute("color"); n)
				ret->color = sto<vec4>(n.value());
			if (auto n = n_material.attribute("metallic"); n)
				ret->metallic = sto<float>(n.value());
			if (auto n = n_material.attribute("roughness"); n)
				ret->roughness = sto<float>(n.value());
			if (auto n = n_material.attribute("alpha_test"); n)
				ret->alpha_test = sto<float>(n.value());
			if (auto n = n_material.attribute("pipeline_file"); n)
				ret->pipeline_file = n.value();
			if (auto n = n_material.attribute("pipeline_defines"); n)
				ret->pipeline_defines = n.value();
			auto i = 0;
			for (auto n_texture : n_material.child("textures"))
			{
				auto& dst = ret->textures[i];
				if (auto n = n_material.attribute("filename"); n)
					dst.filename = n.value();
				if (auto n = n_material.attribute("srgb"); n)
					dst.srgb = n.as_bool();
				if (auto n = n_material.attribute("mag_filter"); n)
					ti_es("flame::graphics::Filter")->unserialize(&dst.mag_filter, n.value());
				if (auto n = n_material.attribute("min_filter"); n)
					ti_es("flame::graphics::Filter")->unserialize(&dst.min_filter, n.value());
				if (auto n = n_material.attribute("linear_mipmap"); n)
					dst.linear_mipmap = n.as_bool();
				if (auto n = n_material.attribute("address_mode"); n)
					ti_es("flame::graphics::AddressMode")->unserialize(&dst.address_mode, n.value());
				i++;
			}
			loaded_materials.emplace_back(ret);
			return ret;
		}

		Material* Material::get(const wchar_t* filename)
		{
			return MaterialPrivate::get(filename);
		}
	}
}
