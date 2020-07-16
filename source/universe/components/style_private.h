#pragma once

#include <flame/universe/components/style.h>

namespace flame
{
	struct TypeInfo* type;
	struct FunctionInfo;

	struct cStyleBridge : cStyle
	{
		void add_rule(StateFlags state, const char* target, const char* variable, const void* value) override;
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

		void add_rule(StateFlags state, const std::string& target, const std::string& variable, const void* value);

		void on_entity_state_changed() override;

		static cStylePrivate* create();
	};

	void cStyleBridge::add_rule(StateFlags state, const char* target, const char* variable, const void* value)
	{
		((cStylePrivate*)this)->add_rule(state, target, variable, value);
	}
}
