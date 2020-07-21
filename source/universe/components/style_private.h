#pragma once

#include <flame/universe/components/style.h>

namespace flame
{
	struct TypeInfo* type;
	struct FunctionInfo;

	struct cStyleBridge : cStyle
	{
		void add_rule(StateFlags state, const char* rule) override;
	};

	struct cStylePrivate : cStyleBridge
	{
		struct Rule
		{
			StateFlags state;
			Component* target;
			TypeInfo* type;
			FunctionInfo* setter;
			void* data = nullptr;

			~Rule()
			{
				type->destroy(data);
			}
		};

		std::vector<std::unique_ptr<Rule>> rules;

		void add_rule(StateFlags state, const std::string& rule);

		void on_entity_state_changed() override;

		static cStylePrivate* create();
	};

	void cStyleBridge::add_rule(StateFlags state, const char* rule)
	{
		((cStylePrivate*)this)->add_rule(state, rule);
	}
}
