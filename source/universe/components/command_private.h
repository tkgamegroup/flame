#pragma once

#include <flame/universe/components/command.h>

namespace flame
{
	struct cCommandPrivate : cCommand
	{
		std::vector<std::unique_ptr<Closure<void(Capture&, const char*)>>> processors;

		void excute(const char* cmd) override;

		void* add_processor(void (*callback)(Capture& c, const char* cmd), const Capture& capture) override;
		void remove_processor(void* p) override;
	};
}
