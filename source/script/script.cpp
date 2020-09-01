#include <flame/foundation/typeinfo.h>
#include "script_private.h"

#include <lua.hpp>

namespace flame
{
	namespace script
	{
		static void print_lua_stack(lua_State* state)
		{
			printf("lua stack:\n");
			auto n = lua_gettop(state);
			for (auto i = 0; i < n; i++)
			{
				auto id = -(i + 1);
				if (lua_isinteger(state, id))
					printf("[%d] - %d\n", id, lua_tointeger(state, id));
				else if (lua_isnumber(state, id))
					printf("[%d] - %f\n", id, (float)lua_tonumber(state, id));
				else if (lua_isstring(state, id))
					printf("[%d] - \"%s\"\n", id, lua_tostring(state, id));
				else if (lua_istable(state, id))
					printf("[%d] - table\n", id);
				else if (lua_iscfunction(state, id))
					printf("[%d] - c function\n", id);
				else if (lua_isuserdata(state, id))
					printf("[%d] - user data\n", id);
				else
					printf("[%d] - unknow\n", id);
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
						else if (tn == "flame::Vec<2,float>" && tt == TypePointer)
						{
							auto pp = *(void**)p;
							if (lua_istable(state, -1))
							{
								lua_pushstring(state, "x");
								lua_gettable(state, -2);
								(*(Vec2f*)pp).x() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);

								lua_pushstring(state, "y");
								lua_gettable(state, -2);
								(*(Vec2f*)pp).y() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);
							}
							else
								*(Vec2f*)p = Vec2f(0.f);
						}
						else if (tn == "flame::Vec<3,float>" && tt == TypePointer)
						{
							auto pp = *(void**)p;
							if (lua_istable(state, -1))
							{
								lua_pushstring(state, "x");
								lua_gettable(state, -2);
								(*(Vec3f*)pp).x() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);

								lua_pushstring(state, "y");
								lua_gettable(state, -2);
								(*(Vec3f*)pp).y() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);

								lua_pushstring(state, "z");
								lua_gettable(state, -2);
								(*(Vec3f*)pp).z() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);
							}
							else
								*(Vec3f*)p = Vec3f(0.f);
						}
						else if (tn == "flame::Vec<4,float>" && tt == TypePointer)
						{
							auto pp = *(void**)p;
							if (lua_istable(state, -1))
							{
								lua_pushstring(state, "x");
								lua_gettable(state, -2);
								(*(Vec4f*)pp).x() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);

								lua_pushstring(state, "y");
								lua_gettable(state, -2);
								(*(Vec4f*)pp).y() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);

								lua_pushstring(state, "z");
								lua_gettable(state, -2);
								(*(Vec4f*)pp).z() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);

								lua_pushstring(state, "w");
								lua_gettable(state, -2);
								(*(Vec4f*)pp).w() = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								lua_pop(state, 1);
							}
							else
								*(Vec4f*)p = Vec4f(0.f);
						}
						lua_pop(state, 1);
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
						InstancePrivate::get()->add_object(*(void**)ret, "staging", tn.c_str());
						lua_getglobal(state, "staging");
					}
					else
					{
						if (tn == "int" || tn == "uint")
							lua_pushinteger(state, *(int*)ret);
						else if (tn == "float")
							lua_pushnumber(state, *(float*)ret);
						else if (tn == "flame::Vec<2,float>")
						{
							lua_newtable(state);

							lua_pushstring(state, "x");
							lua_pushnumber(state, (*(Vec2f*)ret).x());
							lua_settable(state, -3);

							lua_pushstring(state, "y");
							lua_pushnumber(state, (*(Vec2f*)ret).y());
							lua_settable(state, -3);
						}
						else if (tn == "flame::Vec<3,float>")
						{
							lua_newtable(state);

							lua_pushstring(state, "x");
							lua_pushnumber(state, (*(Vec3f*)ret).x());
							lua_settable(state, -3);

							lua_pushstring(state, "y");
							lua_pushnumber(state, (*(Vec3f*)ret).y());
							lua_settable(state, -3);

							lua_pushstring(state, "z");
							lua_pushnumber(state, (*(Vec3f*)ret).z());
							lua_settable(state, -3);
						}
						else if (tn == "flame::Vec<4,float>")
						{
							lua_newtable(state);

							lua_pushstring(state, "x");
							lua_pushnumber(state, (*(Vec4f*)ret).x());
							lua_settable(state, -3);

							lua_pushstring(state, "y");
							lua_pushnumber(state, (*(Vec4f*)ret).y());
							lua_settable(state, -3);

							lua_pushstring(state, "z");
							lua_pushnumber(state, (*(Vec4f*)ret).z());
							lua_settable(state, -3);

							lua_pushstring(state, "w");
							lua_pushnumber(state, (*(Vec4f*)ret).w());
							lua_settable(state, -3);
						}
						else
							lua_pushnil(state);
					}
					ret_type->destroy(ret);
					return 1;
				}
			}
			return 0;
		}

		static int l_hash(lua_State* state)
		{
			if (lua_isstring(state, -1))
			{
				auto hash = std::hash<std::string>()(lua_tostring(state, -1));
				lua_pushlightuserdata(state, (void*)hash);
				return 1;
			}
			return 0;
		}

		InstancePrivate::InstancePrivate()
		{
			lua_state = luaL_newstate();
			luaL_openlibs(lua_state);

			lua_pushcfunction(lua_state, l_call);
			lua_setglobal(lua_state, "flame_call");

			lua_pushcfunction(lua_state, l_hash);
			lua_setglobal(lua_state, "flame_hash");

			if (!excute(L"setup.lua"))
				assert(0);

			lua_newtable(lua_state);
			lua_setglobal(lua_state, "udts");
			traverse_udts([](Capture& c, UdtInfo* ui) {
				auto state = c.thiz<InstancePrivate>()->lua_state;
				auto udt_name = std::string(ui->get_name());
				if (udt_name.ends_with("Private"))
					return;
				lua_newtable(state);
				auto count = ui->get_functions_count();
				for (auto i = 0; i < count; i++)
				{
					auto function = ui->get_function(i);
					lua_pushstring(state, function->get_name());
					lua_pushlightuserdata(state, function);
					lua_settable(state, -3);
				}
				lua_setglobal(state, "udt");

				lua_getglobal(state, "udts");
				lua_pushstring(state, udt_name.c_str());
				lua_getglobal(state, "udt");
				lua_settable(state, -3);
				lua_pop(state, 1);
			}, Capture().set_thiz(this));

			lua_newtable(lua_state);
			lua_setglobal(lua_state, "enums");
			traverse_enums([](Capture& c, EnumInfo* ei) {
				auto state = c.thiz<InstancePrivate>()->lua_state;
				lua_newtable(state);
				auto count = ei->get_items_count();
				for (auto i = 0; i < count; i++)
				{
					auto item = ei->get_item(i);
					lua_pushstring(state, item->get_name());
					lua_pushinteger(state, item->get_value());
					lua_settable(state, -3);
				}
				lua_setglobal(state, "enum");

				lua_getglobal(state, "enums");
				lua_pushstring(state, ei->get_name());
				lua_getglobal(state, "enum");
				lua_settable(state, -3);
				lua_pop(state, 1);
			}, Capture().set_thiz(this));
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
			lua_pushstring(lua_state, type_name);
			check_result(lua_pcall(lua_state, 2, 0, 0));
		}

		void InstancePrivate::call_slot(uint s, uint parameters_count, Parameter* parameters)
		{
			lua_getglobal(lua_state, "slots");
			lua_pushinteger(lua_state, s);
			lua_gettable(lua_state, -2);
			for (auto i = 0; i < parameters_count; i++)
			{
				auto& p = parameters[i];
				switch (p.type)
				{
				case ScriptTypeInt:
					lua_pushinteger(lua_state, *(int*)p.data);
					break;
				case ScriptTypePointer:
					lua_pushlightuserdata(lua_state, *(void**)p.data);
					break;
				}
			}
			check_result(lua_pcall(lua_state, parameters_count, 0, 0));
			lua_pop(lua_state, 1);
		}

		void InstancePrivate::release_slot(uint s)
		{
			lua_getglobal(lua_state, "release_slot");
			lua_pushinteger(lua_state, s);
			check_result(lua_pcall(lua_state, 1, 0, 0));
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
