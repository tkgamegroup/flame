#include <flame/script/script.h>

struct lua_State;

namespace flame
{
	namespace script
	{
		struct InstancePrivate : Instance 
		{
			lua_State* lua_state = nullptr;

			InstancePrivate();

			bool check_result(int res);
		};
	}
}
