#include <flame/universe/components/data_keeper.h>

namespace flame
{
	struct cDataKeeperPrivate : cDataKeeper
	{
		std::map<uint, void*> voidp_datas;
		std::map<uint, std::string> stringa_datas;
	};

	void cDataKeeper::set_voidp_item(uint hash, void* v)
	{
		((cDataKeeperPrivate*)this)->voidp_datas[hash] = v;
	}

	void* cDataKeeper::get_voidp_item(uint hash)
	{
		return ((cDataKeeperPrivate*)this)->voidp_datas[hash];
	}

	void cDataKeeper::remove_voidp_item(uint hash)
	{
		auto& map = ((cDataKeeperPrivate*)this)->voidp_datas;
		map.erase(map.find(hash));
	}

	void cDataKeeper::set_stringa_item(uint hash, const char* v)
	{
		((cDataKeeperPrivate*)this)->stringa_datas[hash] = v;
	}

	const char* cDataKeeper::get_stringa_item(uint hash)
	{
		return ((cDataKeeperPrivate*)this)->stringa_datas[hash].c_str();
	}

	void cDataKeeper::remove_stringa_item(uint hash)
	{
		auto& map = ((cDataKeeperPrivate*)this)->stringa_datas;
		map.erase(map.find(hash));
	}

	cDataKeeper* cDataKeeper::create()
	{
		return new cDataKeeperPrivate();
	}
}
