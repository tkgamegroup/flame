#include "../xml.h"
#include "animation_private.h"

namespace flame
{
	namespace graphics
	{
		static std::vector<std::unique_ptr<AnimationT>> animations;

		void AnimationPrivate::save(const std::filesystem::path& filename)
		{
			pugi::xml_document doc;
			auto n_animation = doc.append_child("animation");
			n_animation.append_attribute("duration").set_value(duration);

			auto data_filename = filename;
			data_filename += L".dat";
			std::ofstream data_file(data_filename, std::ios::binary);

			auto n_channels = n_animation.append_child("channels");
			for (auto& ch : channels)
			{
				auto n_channel = n_channels.append_child("channel");
				n_channel.append_attribute("node_name").set_value(ch.node_name.c_str());
				{
					auto n_keys = n_channel.append_child("position_keys");
					n_keys.append_attribute("offset").set_value(data_file.tellp());
					auto size = sizeof(Channel::PositionKey) * ch.position_keys.size();
					n_keys.append_attribute("size").set_value(size);
					data_file.write((char*)ch.position_keys.data(), size);
				}
				{
					auto n_keys = n_channel.append_child("rotation_keys");
					n_keys.append_attribute("offset").set_value(data_file.tellp());
					auto size = sizeof(Channel::RotationKey) * ch.rotation_keys.size();
					n_keys.append_attribute("size").set_value(size);
					data_file.write((char*)ch.rotation_keys.data(), size);
				}
			}

			data_file.close();

			doc.save_file(filename.c_str());
		}

		struct AnimationGet : Animation::Get
		{
			AnimationPtr operator()(const std::filesystem::path& _filename) override
			{
				auto filename = Path::get(_filename);

				for (auto& a : animations)
				{
					if (a->filename == filename)
						return a.get();
				}

				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("animation"))
				{
					wprintf(L"animation does not exist: %s\n", _filename.c_str());
					return nullptr;
				}

				auto data_filename = filename;
				data_filename += L".dat";
				std::ifstream data_file(data_filename, std::ios::binary);
				if (!data_file.good())
				{
					wprintf(L"missing .dat file for: %s\n", _filename.c_str());
					return nullptr;
				}

				auto ret = new AnimationPrivate;
				ret->duration = doc_root.attribute("duration").as_float();
				ret->filename = filename;

				for (auto n_channel : doc_root.child("channels"))
				{
					auto& c = ret->channels.emplace_back();
					c.node_name = n_channel.attribute("node_name").value();
					{
						auto n_keys = n_channel.child("position_keys");
						auto offset = n_keys.attribute("offset").as_uint();
						auto size = n_keys.attribute("size").as_uint();
						c.position_keys.resize(size / sizeof(Channel::PositionKey));
						data_file.read((char*)c.position_keys.data(), size);
					}
					{
						auto n_keys = n_channel.child("rotation_keys");
						auto offset = n_keys.attribute("offset").as_uint();
						auto size = n_keys.attribute("size").as_uint();
						c.rotation_keys.resize(size / sizeof(Channel::RotationKey));
						data_file.read((char*)c.rotation_keys.data(), size);
					}
				}

				data_file.close();

				ret->ref = 1;
				animations.emplace_back(ret);
				return ret;
			}
		}Animation_get;
		Animation::Get& Animation::get = Animation_get;

		struct AnimationRelease : Animation::Release
		{
			void operator()(AnimationPtr animation) override
			{
				if (animation->ref == 1)
				{
					std::erase_if(animations, [&](const auto& i) {
						return i.get() == animation;
					});
				}
				else
					animation->ref--;
			}
		}Animation_release;
		Animation::Release& Animation::release = Animation_release;
	}
}
