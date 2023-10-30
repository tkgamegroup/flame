#include "../entity_private.h"
#include "graveyard_private.h"

namespace flame
{
	GraveyardPrivate::GraveyardPrivate()
	{
		add_event([this]() { 
			for (auto it = entities.begin(); it != entities.end(); )
			{
				if (it->first > 0)
				{
					it->first -= delta_time;
					++it;
				}
				else
				{
					auto e = it->second;
					if (e->parent)
						e->remove_from_parent();
					else
						delete e;
					it = entities.erase(it);
				}
			}
			return true; 
		});
	}

	void GraveyardPrivate::set_duration(float v)
	{
		if (duration == v)
			return;
		duration = v;
	}

	void GraveyardPrivate::add(EntityPtr e)
	{
		entities.emplace_back(duration, e);
	}

	void GraveyardPrivate::clear()
	{
		entities.clear();
	}

	static GraveyardPtr _instance = nullptr;

	struct GraveyardInstance : Graveyard::Instance
	{
		GraveyardPtr operator()() override
		{
			if (_instance == nullptr)
				_instance = new GraveyardPrivate();
			return _instance;
		}
	}Graveyard_instance;
	Graveyard::Instance& Graveyard::instance = Graveyard_instance;
}
