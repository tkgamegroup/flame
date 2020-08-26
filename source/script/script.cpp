#include <flame/foundation/typeinfo.h>
#include "script_private.h"

#include <lua.hpp>

namespace flame
{
	namespace script
	{
		static void format_type_name(std::string& name)
		{
			for (auto& ch : name)
			{
				if (ch == ':')
					ch = '_';
			}
		}

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

				std::vector<void*> parms;
				parms.resize(f->get_parameters_count());
				for (auto i = 0; i < parms.size(); i++)
				{
					auto type = f->get_parameter(i);
					auto p = type->create();
					if (lua_istable(state, -3))
					{
						auto tt = type->get_tag();
						auto tn = std::string(type->get_name());
						lua_pushinteger(state, i + 1);
						lua_gettable(state, -4);
						if (tn == "int" || tn == "uint")
							*(int*)p = lua_isinteger(state, -1) ? lua_tointeger(state, -1) : -1;
						else if (tn == "float")
							*(float*)p = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
						else if (tn == "char" && tt == TypePointer)
							type->unserialize(p, lua_isstring(state, -1) ? lua_tostring(state, -1) : "");
						else if (tn == "wchar_t" && tt == TypePointer)
							type->unserialize(p, lua_isstring(state, -1) ? lua_tostring(state, -1) : "");
					}
					parms[i] = p;
				}
				f->call(p, ret, parms.data());
				for (auto i = 0; i < parms.size(); i++)
				{
					auto type = f->get_parameter(i);
					type->destroy(parms[i]);
				}

				if (ret)
				{
					auto tn = std::string(ret_type->get_name());
					if (ret_type->get_tag() == TypePointer)
					{
						format_type_name(tn);
						InstancePrivate::get()->add_object(*(void**)ret, "staging", tn.c_str());
						lua_getglobal(state, "staging");
					}
					else
					{
						if (tn == "int" || tn == "uint")
							lua_pushinteger(state, *(int*)ret);
						else if (tn == "float")
							lua_pushnumber(state, *(float*)ret);
						else
							lua_pushnil(state);
					}
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

			if (excute(L"setup.lua"))
			{
				traverse_udts([](Capture& c, UdtInfo* udt) {
					auto state = c.thiz<InstancePrivate>()->lua_state;
					auto udt_name = std::string(udt->get_name());
					if (udt_name.ends_with("Private"))
						return;
					format_type_name(udt_name);
					lua_newtable(state);
					auto count = udt->get_functions_count();
					for (auto i = 0; i < count; i++)
					{
						auto f = udt->get_function(i);
						lua_pushstring(state, f->get_name());
						lua_pushlightuserdata(state, f);
						lua_settable(state, -3);
					}
					lua_setglobal(state, udt_name.c_str());
				}, Capture().set_thiz(this));
			}
		}

		bool InstancePrivate::check_result(int res)
		{
			if (res != LUA_OK)
			{
				printf("%s\n", lua_tostring(lua_state, -1));
				return false;
			}
			return true;
		}

		bool InstancePrivate::excute(const std::filesystem::path& filename)
		{
			auto path = filename;
			if (!std::filesystem::exists(path))
			{
				auto engine_path = getenv("FLAME_PATH");
				if (engine_path)
					path = std::filesystem::path(engine_path) / "assets" / path;
			}
			return check_result(luaL_dofile(lua_state, path.string().c_str()));
		}

		void InstancePrivate::add_object(void* p, const char* name, const char* type_name)
		{
			lua_newtable(lua_state);
			lua_pushstring(lua_state, "p");
			lua_pushlightuserdata(lua_state, p);
			lua_settable(lua_state, -3);
			lua_setglobal(lua_state, name);

			lua_getglobal(lua_state, "make_obj");
			lua_getglobal(lua_state, name);
			lua_getglobal(lua_state, type_name);
			check_result(lua_pcall(lua_state, 2, 0, 0));
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
