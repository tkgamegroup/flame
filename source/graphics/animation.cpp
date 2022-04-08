#include "../serialize_extension.h"
#include "animation_private.h"

namespace flame
{
	namespace graphics
	{
		static std::vector<std::unique_ptr<AnimationT>> animations;

		void AnimationPrivate::save(const std::filesystem::path& filename)
		{
			std::ofstream dst(filename);
			dst << "animation:" << std::endl;

			pugi::xml_document doc;
			auto n_animation = doc.append_child("animation");
			n_animation.append_attribute("duration").set_value(duration);

			DataSoup data_soup;

			auto n_channels = n_animation.append_child("channels");
			for (auto& ch : channels)
			{
				auto n_channel = n_channels.append_child("channel");
				n_channel.append_attribute("node_name").set_value(ch.node_name.c_str());
				data_soup.xml_append_v(ch.position_keys, n_channel.append_child("position_keys"));
				data_soup.xml_append_v(ch.rotation_keys, n_channel.append_child("rotation_keys"));
			}

			doc.save(dst);
			dst << std::endl;

			dst << "data:" << std::endl;
			data_soup.save(dst);
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

				std::ifstream file(filename);
				LineReader src(file);
				src.read_block("animation:");

				pugi::xml_document doc;
				pugi::xml_node doc_root;
				if (!doc.load_string(src.to_string().c_str()) || (doc_root = doc.first_child()).name() != std::string("animation"))
				{
					wprintf(L"animation format is incorrect: %s\n", _filename.c_str());
					return nullptr;
				}

				DataSoup data_soup;
				src.read_block("data:");
				data_soup.load(src);

				auto ret = new AnimationPrivate;
				ret->duration = doc_root.attribute("duration").as_float();
				ret->filename = filename;

				for (auto n_channel : doc_root.child("channels"))
				{
					auto& c = ret->channels.emplace_back();
					c.node_name = n_channel.attribute("node_name").value();
					data_soup.xml_read_v(c.position_keys, n_channel.child("position_keys"));
					data_soup.xml_read_v(c.rotation_keys, n_channel.child("rotation_keys"));
				}

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
