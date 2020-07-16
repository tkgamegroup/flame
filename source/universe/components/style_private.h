#pragma once

#include <flame/universe/components/style.h>

namespace flame
{
	struct TypeInfo* type;
	struct FunctionInfo;

	struct cStyleBridge : cStyle
	{

	};

	struct cStylePrivate : cStyleBridge
	{
		struct Rule
		{
			StateFlags state;
			Component* target;
			TypeInfo* type;
			FunctionInfo* setter;
			char* data = nullptr;

			~Rule()
			{
				delete[]data;
			}
		};

		std::vector<std::unique_ptr<Rule>> rules;
	};
}
