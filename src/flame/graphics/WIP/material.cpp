#include <flame/global.h>
#include <flame/engine/graphics/resource.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/sampler.h>
#include <flame/engine/graphics/material.h>

namespace flame
{
	struct MaterialShaderStruct
	{
		union
		{
			struct
			{
				unsigned char albedo_r;
				unsigned char albedo_g;
				unsigned char albedo_b;
				unsigned char alpha;
			};
			unsigned int packed;
		}albedo_alpha;

		union
		{
			struct
			{
				unsigned char spec;
				unsigned char roughness;
				unsigned char dummy0;
				unsigned char dummy1;
			};
			unsigned int packed;
		}spec_roughness;

		union
		{
			struct
			{
				unsigned char albedo_alpha;
				unsigned char normal_height;
				unsigned char spec_roughness;
				unsigned char dummy;
			};
			unsigned int packed;
		}map_index;

		unsigned int dummy;
	};

	static void _update_material(Material *m)
	{
		defalut_staging_buffer->map(0, sizeof(MaterialShaderStruct));
		auto map = (unsigned char*)defalut_staging_buffer->mapped;
		MaterialShaderStruct stru;
		auto albedo_alpha = glm::clamp(m->get_albedo_alpha(), 0.f, 1.f);
		stru.albedo_alpha.albedo_r = albedo_alpha.r * 255.f;
		stru.albedo_alpha.albedo_g = albedo_alpha.g * 255.f;
		stru.albedo_alpha.albedo_b = albedo_alpha.b * 255.f;
		stru.albedo_alpha.alpha = albedo_alpha.a * 255.f;
		stru.spec_roughness.spec = glm::clamp(m->get_spec(), 0.f, 1.f) * 255.f;
		stru.spec_roughness.roughness = glm::clamp(m->get_roughness(), 0.f, 1.f) * 255.f;
		auto albedo_alpha_map = m->get_albedo_alpha_map();
		auto spec_roughness_map = m->get_spec_roughness_map();
		auto normal_height_map = m->get_normal_height_map();
		stru.map_index.albedo_alpha = albedo_alpha_map ? albedo_alpha_map->material_index + 1 : 0;
		stru.map_index.spec_roughness = spec_roughness_map ? spec_roughness_map->material_index + 1 : 0;
		stru.map_index.normal_height = normal_height_map ? normal_height_map->material_index + 1 : 0;
		memcpy(map, &stru, sizeof(MaterialShaderStruct));
		defalut_staging_buffer->unmap();

		VkBufferCopy range = {};
		range.srcOffset = 0;
		range.dstOffset = sizeof(MaterialShaderStruct) * m->get_index();
		range.size = sizeof(MaterialShaderStruct);

		defalut_staging_buffer->copy_to(materialBuffer, 1, &range);
	}

	Material::Material() :
		albedo_alpha(1.f),
		spec_roughness(0.f, 1.f),
		index(-1)
	{
	}

	std::string Material::get_name() const
	{
		return name;
	}

	glm::vec4 Material::get_albedo_alpha() const
	{
		return albedo_alpha;
	}

	float Material::get_spec() const
	{
		return spec_roughness.x;
	}

	float Material::get_roughness() const
	{
		return spec_roughness.y;
	}

	Texture *Material::get_albedo_alpha_map() const
	{
		return albedo_alpha_map.get();
	}

	Texture *Material::get_spec_roughness_map() const
	{
		return spec_roughness_map.get();
	}

	Texture *Material::get_normal_height_map() const
	{
		return normal_height_map.get();
	}

	std::string Material::get_albedo_alpha_map_name() const
	{
		if (albedo_alpha_map)
			return albedo_alpha_map->filename;
		return "";
	}

	std::string Material::get_spec_roughness_map_name() const
	{
		if (spec_roughness_map)
			return spec_roughness_map->filename;
		return "";
	}

	std::string Material::get_normal_height_map_name() const
	{
		if (normal_height_map)
			return normal_height_map->filename;
		return "";
	}

	int Material::get_index() const
	{
		return index;
	}

	void Material::set_name(const std::string &v)
	{
		name = v;
	}

	void Material::set_albedo_alpha(const glm::vec4 &v)
	{
		albedo_alpha = v;
		_update_material(this);
	}

	void Material::set_spec(float v)
	{
		spec_roughness.x = v;
		_update_material(this);
	}

	void Material::set_roughness(float v)
	{
		spec_roughness.y = v;
		_update_material(this);
	}

	void Material::set_albedo_alpha_map(const std::string &filename)
	{
		if (filename == "")
			albedo_alpha_map.reset();
		else
		{
			if (albedo_alpha_map && albedo_alpha_map->filename == filename)
				return;

			albedo_alpha_map = getMaterialImage(filename);
		}
		_update_material(this);
	}

	void Material::set_spec_roughness_map(const std::string &filename)
	{
		if (filename == "")
			spec_roughness_map.reset();
		else
		{
			if (spec_roughness_map && spec_roughness_map->filename == filename)
				return;

			spec_roughness_map = getMaterialImage(filename);
		}
		_update_material(this);
	}

	void Material::set_normal_height_map(const std::string &filename)
	{
		if (filename == "")
			normal_height_map.reset();
		else
		{
			if (normal_height_map && normal_height_map->filename == filename)
				return;

			normal_height_map = getMaterialImage(filename);
		}
		_update_material(this);
	}

	void Material::set_index(int v)
	{
		index = v;
	}

	static SpareList _material_list(MaxMaterialCount);
	static std::weak_ptr<Material> _materials[MaxMaterialCount];
	std::shared_ptr<Material> default_material;
	Buffer *materialBuffer = nullptr;

	std::shared_ptr<Material> getMaterial(const glm::vec4 &albedo_alpha, float spec, float roughness,
		const std::string &albedo_alpha_map_filename, const std::string &spec_roughness_map_filename,
		const std::string &normal_height_map_filename)
	{
		std::shared_ptr<Material> m;
		_material_list.iterate([&](int index, void *p, bool &remove) {
			auto _m = _materials[index].lock();
			if (_m)
			{
				if (is_same(_m->get_albedo_alpha(), albedo_alpha) &&
					is_same(_m->get_spec(), spec) &&
					is_same(_m->get_roughness(), roughness) &&
					_m->get_albedo_alpha_map_name() == albedo_alpha_map_filename &&
					_m->get_spec_roughness_map_name() == spec_roughness_map_filename &&
					_m->get_normal_height_map_name() == normal_height_map_filename)
				{
					m = _m;
					return false;
				}
			}
			else
			{
				_materials[index].reset();
				remove = true;
			}
			return true;
		});

		if (!m)
		{
			m = std::make_shared<Material>();
			m->set_albedo_alpha(albedo_alpha);
			m->set_spec(spec);
			m->set_roughness(roughness);
			m->set_albedo_alpha_map(albedo_alpha_map_filename);
			m->set_spec_roughness_map(spec_roughness_map_filename);
			m->set_normal_height_map(normal_height_map_filename);
			auto index = _material_list.add(m.get());
			m->set_index(index);
			_materials[index] = m;
			_update_material(m.get());
		}

		return m;
	}

	std::shared_ptr<Material> getMaterial(const std::string name)
	{
		std::shared_ptr<Material> m;
		_material_list.iterate([&](int index, void *p, bool &remove) {
			auto _m = _materials[index].lock();
			if (_m)
			{
				if (_m->get_name() == name)
				{
					m = _m;
					return false;
				}
			}
			else
			{
				_materials[index].reset();
				remove = true;
			}
			return true;
		});

		return m ? m : default_material;
	}

	static SpareList _material_image_list(MaxMaterialImageCount);
	static std::weak_ptr<Texture> _material_images[MaxMaterialImageCount];

	std::shared_ptr<Texture> getMaterialImage(const std::string &_filename)
	{
		if (_filename == "")
			return nullptr;

		std::shared_ptr<Texture> i;
		_material_image_list.iterate([&](int index, void *p, bool &remove) {
			auto _i = _material_images[index].lock();
			if (_i)
			{
				if (_i->filename == _filename)
				{
					i = _i;
					return false;
				}
			}
			else
			{
				_material_images[index].reset();
				remove = true;
			}
			return true;
		});

		if (!i)
		{
			i = get_texture(_filename);
			if (!i)
				return nullptr;

			auto index = _material_image_list.add(i.get());
			i->material_index = index;
			_material_images[index] = i;

			updateDescriptorSets(&ds_material->get_write(MaterialImagesDescriptorBinding, index, &get_texture_info(i.get(), colorSampler)));
		}

		return i;
	}

	DescriptorSet *ds_material = nullptr;

	static std::shared_ptr<DescriptorSetLayout> _material_layout;

	void init_material()
	{
		std::vector<DescriptorSetLayoutBinding> bindings = {
			{
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				MaterialBufferDescriptorBinding,
				1,
				VK_SHADER_STAGE_FRAGMENT_BIT
			},
			{
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				MaterialImagesDescriptorBinding,
				MaxMaterialCount,
				VK_SHADER_STAGE_FRAGMENT_BIT
			}
		};
		_material_layout = get_descriptor_set_layout(bindings);

		ds_material = new DescriptorSet(_material_layout.get());

		materialBuffer = new Buffer(BufferTypeUniform, sizeof(MaterialShaderStruct) * MaxMaterialCount);

		default_material = std::make_shared<Material>();
		default_material->set_name("[default_material]");
		default_material->set_index(0);
		{
			auto index = _material_list.add(default_material.get());
			_materials[index] = default_material;
		}
		_update_material(default_material.get());

		updateDescriptorSets(&ds_material->get_write(MaterialBufferDescriptorBinding, 0, &get_buffer_info(materialBuffer)));

		for (auto i = 0; i < MaxMaterialImageCount; i++)
			updateDescriptorSets(&ds_material->get_write(MaterialImagesDescriptorBinding, i, &get_texture_info(default_color_texture.get(), colorSampler)));
	}
}
