#pragma once

#include <string>
#include <memory>

#include <flame/math.h>
#include <flame/spare_list.h>

namespace flame
{
	enum { MaxMaterialCount = 256 };
	enum { MaxMaterialImageCount = 256 };

	struct Buffer;
	struct Texture;
	struct DescriptorSet;

	class Material
	{
	private:
		std::string name;

		glm::vec4 albedo_alpha;
		glm::vec2 spec_roughness;

		std::shared_ptr<Texture> albedo_alpha_map;
		std::shared_ptr<Texture> spec_roughness_map;
		std::shared_ptr<Texture> normal_height_map;

		int index = -1;
	public:
		Material();

		std::string get_name() const;
		glm::vec4 get_albedo_alpha() const;
		float get_spec() const;
		float get_roughness() const;
		Texture *get_albedo_alpha_map() const;
		Texture *get_spec_roughness_map() const;
		Texture *get_normal_height_map() const;
		std::string get_albedo_alpha_map_name() const;
		std::string get_spec_roughness_map_name() const;
		std::string get_normal_height_map_name() const;
		int get_index() const;

		void set_name(const std::string &v);
		void set_albedo_alpha(const glm::vec4 &v);
		void set_spec(float v);
		void set_roughness(float v);
		void set_albedo_alpha_map(const std::string &filename);
		void set_spec_roughness_map(const std::string &filename);
		void set_normal_height_map(const std::string &filename);
		void set_index(int v);
	};

	extern std::shared_ptr<Material> default_material;
	extern Buffer *materialBuffer;
	std::shared_ptr<Material> getMaterial(const glm::vec4 &albedo_alpha, float spec, float roughness,
		const std::string &albedo_alpha_map_filename, const std::string &spec_roughness_map_filename,
		const std::string &normal_height_map_filename);
	std::shared_ptr<Material> getMaterial(const std::string name);

	std::shared_ptr<Texture> getMaterialImage(const std::string &filename);

	extern DescriptorSet *ds_material;

	void init_material();
}
