#include "../../../foundation/blueprint.h"
#include "../../entity_private.h"
#include "../../components/audio_source_private.h"

namespace flame
{
	void add_audio_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("Audio Source Add Buffer", "",
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
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto& path = *(std::filesystem::path*)inputs[1].data;
					auto& name = *(std::string*)inputs[2].data;
					if (!path.empty() && !name.empty())
					{
						auto audio_source = entity->get_component<cAudioSource>();
						if (audio_source)
							audio_source->add_buffer_name(path, name);
					}
				}
			}
		);

		library->add_template("Audio Source Play", "",
			{
				{
					.name = "Entity",
					.allowed_types = { TypeInfo::get<EntityPtr>() }
				},
				{
					.name = "Name_hash",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Volumn",
					.allowed_types = { TypeInfo::get<float>() },
					.default_value = "1"
				}
			},
			{
			},
			[](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto entity = *(EntityPtr*)inputs[0].data;
				if (entity)
				{
					auto name = *(uint*)inputs[1].data;
					auto volumn = *(float*)inputs[2].data;
					auto audio_source = entity->get_component<cAudioSource>();
					if (audio_source)
						audio_source->play(name, volumn);
				}
			}
		);
	}
}
