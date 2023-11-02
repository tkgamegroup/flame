#include "audio_source_private.h"
#include "node_private.h"
#if USE_AUDIO_MODULE
#include "../../audio/buffer.h"
#include "../../audio/source.h"
#endif

namespace flame
{
	void cAudioSourcePrivate::set_buffer_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names)
	{
#if USE_AUDIO_MODULE
		if (buffer_names == names)
			return;

		for (auto& n : buffer_names)
		{
			if (!n.first.empty())
				AssetManagemant::release(Path::get(n.first));
		}
		for (auto& src : sources)
		{
			audio::Buffer::release(src.second.buf);
			delete src.second.src;
		}
		sources.clear();
		buffer_names = names;
		for (auto& n : buffer_names)
		{
			if (!n.first.empty())
				AssetManagemant::get(Path::get(n.first));
		}

		for (auto& n : buffer_names)
		{
			if (!n.first.empty())
			{
				if (auto buf = audio::Buffer::get(n.first); buf)
				{
					auto& src = sources[sh(n.second.c_str())];
					src.buf = buf;
					src.src = audio::Source::create();
					src.src->add_buffer(buf);
				}
			}
		}

		node->mark_transform_dirty();
		data_changed("buffer_names"_h);
#endif
	}

	void cAudioSourcePrivate::add_buffer_name(const std::filesystem::path& path, const std::string& name)
	{
#if USE_AUDIO_MODULE
		auto hash = sh(name.c_str());
		if (sources.find(hash) != sources.end())
		{
			printf("cAudioSource add buffer name: %s already existed\n", name.c_str());
			return;
		}

		if (!path.empty())
		{
			if (auto buf = audio::Buffer::get(path); buf)
			{
				auto& src = sources[hash];
				src.buf = buf;
				src.src = audio::Source::create();
				src.src->add_buffer(buf);
			}
		}
#endif
	}

	cAudioSourcePrivate::~cAudioSourcePrivate()
	{
		for (auto& n : buffer_names)
		{
			if (!n.first.empty())
				AssetManagemant::release(Path::get(n.first));
		}
#if USE_AUDIO_MODULE
		for (auto& src : sources)
		{
			audio::Buffer::release(src.second.buf);
			delete src.second.src;
		}
#endif
	}

	void cAudioSourcePrivate::play(uint name, float volumn)
	{
#if USE_AUDIO_MODULE
		auto it = sources.find(name);
		if (it != sources.end())
		{
			it->second.src->set_volumn(volumn);
			it->second.src->play();
		}
#endif
	}

	void cAudioSourcePrivate::stop(uint name)
	{
#if USE_AUDIO_MODULE
		auto it = sources.find(name);
		if (it != sources.end())
			it->second.src->stop();
#endif
	}

	void cAudioSourcePrivate::pause(uint name)
	{
#if USE_AUDIO_MODULE
		auto it = sources.find(name);
		if (it != sources.end())
			it->second.src->pause();
#endif
	}

	void cAudioSourcePrivate::rewind(uint name)
	{
#if USE_AUDIO_MODULE
		auto it = sources.find(name);
		if (it != sources.end())
			it->second.src->rewind();
#endif
	}

	void cAudioSourcePrivate::update()
	{
#if USE_AUDIO_MODULE
		auto pos = node->global_pos();
		for (auto& src : sources)
			src.second.src->set_pos(pos);
#endif
	}

	struct cAudioSourceCreate : cAudioSource::Create
	{
		cAudioSourcePtr operator()(EntityPtr e) override
		{
			return new cAudioSourcePrivate();
		}
	}cAudioSource_create;
	cAudioSource::Create& cAudioSource::create = cAudioSource_create;
}
