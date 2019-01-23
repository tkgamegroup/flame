#pragma once

#include <memory>

namespace flame
{
	struct Texture;
	struct Material;
}

struct MaterialShower
{
	std::shared_ptr<flame::Texture> empty_image;
	std::shared_ptr<flame::Texture> albedo_alpha_image;
	std::shared_ptr<flame::Texture> spec_roughness_image;
	std::shared_ptr<flame::Texture> normal_height_image;

	flame::Material *m;

	MaterialShower(flame::Material *_m);
	~MaterialShower();
	void show();
};
