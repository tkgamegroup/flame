#include "nav_agent_private.h"

namespace flame
{
	struct cNavAgentCreate : cNavAgent::Create
	{
		cNavAgentPtr operator()() override
		{
			return new cNavAgentPrivate();
		}
	}cNavAgent_create_private;
	cNavAgent::Create& cNavAgent::create = cNavAgent_create_private;
}
