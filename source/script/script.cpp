#include "script_private.h"

#include <lua.hpp>

namespace flame
{
	namespace script
	{
		InstancePrivate::InstancePrivate()
		{
			lua_state = luaL_newstate();

			//lua_newuserdata(lua_state, sizeof(void*));

			//lua_newtable(lua_state);
			//luaL_setfuncs(lua_state, [], 0);
			//lua_setglobal(lua_state, "");
		}
	}
}
