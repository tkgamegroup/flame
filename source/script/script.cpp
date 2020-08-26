#include <flame/foundation/typeinfo.h>
#include "script_private.h"

#include <lua.hpp>

namespace flame
{
	namespace script
	{
		static int l_call(lua_State* state)
		{
			if (lua_isuserdata(state, -1) && lua_isuserdata(state, -2))
			{
				auto f = (FunctionInfo*)lua_touserdata(state, -1);
				auto p = lua_touserdata(state, -2);

				void* ret = nullptr;
				auto ret_type = f->get_type();
				if (ret_type != TypeInfo::get(TypeData, "void"))
					ret = ret_type->create();

				f->call(p, ret, {});

				if (ret)
				{
					auto tn = std::string(ret_type->get_name());
					if (tn == "int" || tn == "uint")
						lua_pushinteger(state, *(int*)ret);
					else if (tn == "float")
						lua_pushnumber(state, *(float*)ret);
					else
						lua_pushnil(state);
					ret_type->destroy(ret);
					return 1;
				}
			}
			return 0;
		}

		InstancePrivate::InstancePrivate()
		{
			lua_state = luaL_newstate();
			luaL_openlibs(lua_state);

			lua_pushcfunction(lua_state, l_call);
			lua_setglobal(lua_state, "flame_call");

			if (check_result(luaL_dofile(lua_state, "d:/flame/assets/setup.lua")))
			{
				auto udt = find_udt("flame::cElement");

				void* c;
				{
					auto fc = udt->find_function("create");
					if (fc->get_type()->get_tag() == TypePointer && fc->get_parameters_count() == 0)
						fc->call(nullptr, &c, {});
				}
				{
					auto f = udt->find_function("set_x");
					float x = 100.f;
					void* ps[] = {
						&x
					};
					f->call(c, nullptr, ps);
				}

				lua_newtable(lua_state);
				lua_pushstring(lua_state, "get_x");
				lua_pushlightuserdata(lua_state, udt->find_function("get_x"));
				lua_settable(lua_state, -3);
				lua_setglobal(lua_state, "fs");

				lua_newtable(lua_state);
				lua_pushstring(lua_state, "p");
				lua_pushlightuserdata(lua_state, c);
				lua_settable(lua_state, -3);
				lua_setglobal(lua_state, "entity");

				lua_getglobal(lua_state, "make_obj");
				lua_getglobal(lua_state, "entity");
				lua_getglobal(lua_state, "fs");
				if (check_result(lua_pcall(lua_state, 2, 0, 0)))
				{
					check_result(luaL_dofile(lua_state, "d:/2.lua"));
				}
			}
		}

		bool InstancePrivate::check_result(int res)
		{
			if (res != LUA_OK)
			{
				printf(lua_tostring(lua_state, -1));
				return false;
			}
			return true;
		}

		static InstancePrivate* instance = nullptr;

		Instance* Instance::get()
		{
			if (!instance)
				instance = new InstancePrivate;
			return instance;
		}
	}
}
