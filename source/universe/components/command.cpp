#include "../entity_private.h"
#include "command_private.h"

namespace flame
{
	void cCommandPrivate::excute(const char* cmd)
	{
		for (auto& p : processors)
			p->call(cmd);
	}

	void* cCommandPrivate::add_processor(void (*callback)(Capture& c, const char* cmd), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		processors.emplace_back(c);
		return c;
	}

	void cCommandPrivate::remove_processor(void* lis)
	{
		std::erase_if(processors, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	cCommand* cCommand::create()
	{
		return f_new<cCommandPrivate>();
	}
}
