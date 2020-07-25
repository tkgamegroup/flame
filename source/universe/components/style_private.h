#pragma once

#include <flame/universe/components/style.h>

namespace flame
{
	struct TypeInfo* type;
	struct FunctionInfo;

	struct cStyleBridge : cStyle
	{
		void set_rule(const char* rule) override;
	};

	struct cStylePrivate : cStyleBridge
	{
		struct Command
		{
			TypeInfo* type;
			FunctionInfo* setter;
			void* data = nullptr;

			~Command()
			{
				type->destroy(data);
			}
		};

		struct Situation
		{
			StateFlags s;
			std::vector<std::unique_ptr<Command>> cmds;
		};

		struct Target
		{
			Component* c;
			std::vector<Situation> situations;
		};

		std::vector<Target> targets;
		std::string rule;

		const char* get_rule() const override { return rule.c_str(); }
		void set_rule(const std::string& rule);

		void on_entity_state_changed() override;
	};

	void cStyleBridge::set_rule(const char* rule)
	{
		((cStylePrivate*)this)->set_rule(rule);
	}
}
