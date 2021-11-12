#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "image_private.h"
#include "material_private.h"

namespace flame
{
	namespace graphics
	{
		MaterialPrivate* default_material = new MaterialPrivate;
		static std::vector<std::unique_ptr<MaterialPrivate>> materials;

		void MaterialPrivate::get_texture_file(uint idx, wchar_t* dst) const
		{
			auto& src = textures[idx];
			if (src.filename.empty())
				dst[0] = 0;
			else
			{
				auto path = filename.parent_path() / src.filename;
				if (!std::filesystem::exists(path))
				{
					path = src.filename;
					get_engine_path(path, L"assets");
				}
				wcscpy(dst, path.c_str());
			}
		}

		SamplerPtr MaterialPrivate::get_texture_sampler(DevicePtr device, uint idx) const
		{
			auto& src = textures[idx];
			return SamplerPrivate::get(device, src.mag_filter, src.min_filter, src.linear_mipmap, src.address_mode);
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
				ret->color = sto<4, float>(n.value());
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

			auto ti_Filter = TypeInfo::get(TypeEnumSingle, "flame::graphics::Filter", tidb);
			auto ti_addrmod = TypeInfo::get(TypeEnumSingle, "flame::graphics::AddressMode", tidb);
			auto i = 0;
			for (auto n_texture : n_material)
			{
				auto& dst = ret->textures[i];
				if (auto n = n_texture.attribute("filename"); n)
					dst.filename = n.value();
				if (auto n = n_texture.attribute("srgb"); n)
					dst.srgb = n.as_bool();
				if (auto n = n_texture.attribute("mag_filter"); n)
					ti_Filter->unserialize(n.value(), &dst.mag_filter);
				if (auto n = n_texture.attribute("min_filter"); n)
					ti_Filter->unserialize(n.value(), &dst.min_filter);
				if (auto n = n_texture.attribute("linear_mipmap"); n)
					dst.linear_mipmap = n.as_bool();
				if (auto n = n_texture.attribute("address_mode"); n)
					ti_addrmod->unserialize(n.value(), &dst.address_mode);
				if (auto n = n_texture.attribute("auto_mipmap"); n)
					dst.auto_mipmap = n.as_bool();
				i++;
			}
			materials.emplace_back(ret);
			return ret;
		}

		Material* Material::get(const wchar_t* filename)
		{
			return MaterialPrivate::get(filename);
		}
	}
}
