#include "../../../foundation/blueprint.h"
#include "../../../graphics/image.h"
#include "../../entity_private.h"
#include "../../components/resources_holder_private.h"

namespace flame
{
	void add_resource_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Hold Resource", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Path",
					.allowed_types = { TypeInfo::get<std::filesystem::path>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto holder = entity->get_component<cResourcesHolder>();
					if (holder)
					{
						auto& path = *(std::filesystem::path*)inputs[1].data;
						auto name = *(uint*)inputs[2].data;
						if (!path.empty() && name)
							holder->hold(path, name);
					}
				}
			}
		);

		library->add_template("Get Image Resource", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Image",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& image = *(graphics::ImageDesc*)outputs[0].data;
				image.view = nullptr;
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto holder = entity->get_component<cResourcesHolder>(); holder)
					{
						if (auto name = *(uint*)inputs[1].data; name)
						{
							if (auto r = holder->get_graphics_image(name); r)
							{
								image.view = r->get_view();
								image.uvs = vec4(0.f, 0.f, 1.f, 1.f);
								graphics::ImageConfig config;
								graphics::Image::get_config(r->filename, &config);
								image.border = config.border;
							}
						}
					}
				}
			}
		);

		library->add_template("Get Image Atlas Resource", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
				{
					.name = "Atlas",
					.allowed_types = { TypeInfo::get<graphics::ImageAtlasPtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto& atlas = *(graphics::ImageAtlasPtr*)outputs[0].data;
				atlas = nullptr;
				if (auto entity = *(EntityPtr*)inputs[0].data; entity)
				{
					if (auto holder = entity->get_component<cResourcesHolder>(); holder)
					{
						if (auto name = *(uint*)inputs[1].data; name)
						{
							auto r = holder->get_graphics_image_atlas(name);
							atlas = r;
						}
					}
				}
			}
		);

		library->add_template("Image Atlas Item", "",
			{
				{
					.name = "Atlas",
					.allowed_types = { TypeInfo::get<graphics::ImageAtlasPtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>(), TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "Item",
					.allowed_types = { TypeInfo::get<graphics::ImageDesc>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto atlas = *(graphics::ImageAtlasPtr*)inputs[0].data;
				auto& item = *(graphics::ImageDesc*)outputs[0].data;
				if (atlas)
				{
					auto name = *(uint*)inputs[1].data;
					if (name)
						item = atlas->get_item(name);
				}
			}
		);
	}
}
