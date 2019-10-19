#include <flame/foundation/foundation.h>
#include "universe_private.h"

namespace flame
{
	void* universe_alloc(uint size)
	{
		return malloc(size);
	}

	void* add_listener_plain(void* hub, void(*pf)(void* c), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c)>;
		c->function = pf;
		c->capture = capture;
		((ListenerHub*)hub)->listeners.emplace_back(c);
		return c;
	}

	void remove_listener_plain(void* hub, void* c)
	{
		auto& listeners = ((ListenerHub*)hub)->listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == c)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	static std::map<std::string, void*> serialization_datas;

	void universe_serialization_initialize()
	{
		serialization_datas.clear();
	}

	void universe_serialization_set_data(const std::string& name, void* data)
	{
		serialization_datas[name] = data;
	}

	void* universe_serialization_get_data(const std::string& name)
	{
		return serialization_datas[name];
	}

	const std::string& universe_serialization_find_data(void* data)
	{
		for (auto it = serialization_datas.begin(); it != serialization_datas.end(); it++)
		{
			if (it->second == data)
				return it->first;
		}

		return "";
	}
}
