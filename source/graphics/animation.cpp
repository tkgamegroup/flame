#include "../xml.h"
#include "animation_private.h"

namespace flame
{
	namespace graphics
	{
		static std::vector<std::unique_ptr<AnimationT>> animations;

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
				ret->filename = filename;

				for (auto n_channel : doc_root.child("channels"))
				{
					auto& c = ret->channels.emplace_back();
					c.node_name = n_channel.attribute("node_name").value();
					{
						auto n_keys = n_channel.child("keys");
						auto offset = n_keys.attribute("offset").as_uint();
						auto size = n_keys.attribute("size").as_uint();
						c.keys.resize(size / sizeof(Channel::Key));
						data_file.read((char*)c.keys.data(), size);
					}
					ret->channels.emplace_back(c);
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
