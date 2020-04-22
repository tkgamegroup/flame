#include <flame/universe/components/data_keeper.h>

namespace flame
{
	struct cDataKeeperPrivate : cDataKeeper
	{
		std::map<uint, CommonValue> common_datas;
		std::map<uint, std::string> string_datas;
	};

	void cDataKeeper::set_common_item(uint hash, const CommonValue& v)
	{
		((cDataKeeperPrivate*)this)->common_datas[hash] = v;
	}

	CommonValue cDataKeeper::get_common_item(uint hash)
	{
		return ((cDataKeeperPrivate*)this)->common_datas[hash];
	}

	void cDataKeeper::remove_common_item(uint hash)
	{
		auto& map = ((cDataKeeperPrivate*)this)->common_datas;
		map.erase(map.find(hash));
	}

	void cDataKeeper::set_string_item(uint hash, const char* v)
	{
		((cDataKeeperPrivate*)this)->string_datas[hash] = v;
	}

	const char* cDataKeeper::get_string_item(uint hash)
	{
		return ((cDataKeeperPrivate*)this)->string_datas[hash].c_str();
	}

	void cDataKeeper::remove_string_item(uint hash)
	{
		auto& map = ((cDataKeeperPrivate*)this)->string_datas;
		map.erase(map.find(hash));
	}

	cDataKeeper* cDataKeeper::create()
	{
		return new cDataKeeperPrivate();
	}
}
