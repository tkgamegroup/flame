#include <flame/foundation/typeinfo.h>
#include "script_private.h"

#include <lua.hpp>

namespace flame
{
	namespace script
	{
		thread_local InstancePrivate* default_instance = nullptr;

		bool lua_check_result(lua_State* state, int res)
		{
			if (res != LUA_OK)
			{
				printf("%s\n", lua_tostring(state, -1));
				return false;
			}
			return true;
		}

		template <uint N>
		void lua_push(lua_State* state, const vec<N, float>& v)
		{
			const char* names[] = {
				"x", "y", "z", "w"
			};

			lua_newtable(state);

			for (auto i = 0; i < N; i++)
			{
				lua_pushstring(state, names[i]);
				lua_pushnumber(state, v[i]);
				lua_settable(state, -3);
			}
		}

		template <uint N>
		vec<N, float> lua_pull(lua_State* state)
		{
			const char* names[] = {
				"x", "y", "z", "w"
			};

			vec<N, float> ret(0.f);
			if (lua_istable(state, -1))
			{
				for (auto i = 0; i < N; i++)
				{
					lua_pushstring(state, names[i]);
					lua_gettable(state, -2);
					ret[i] = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
					lua_pop(state, 1);
				}
			}
			return ret;
		}

		void lua_set_object_type(lua_State* state, const char* type_name)
		{
			lua_getglobal(state, "make_obj");
			lua_pushvalue(state, -2);
			lua_pushstring(state, type_name);
			lua_check_result(state, lua_pcall(state, 2, 0, 0));
		}

		void lua_print_stack(lua_State* state)
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

		int lua_flame_call(lua_State* state)
		{
			auto f = lua_isuserdata(state, -1) ? (FunctionInfo*)lua_touserdata(state, -1) : nullptr;
			auto p = lua_isuserdata(state, -2) ? lua_touserdata(state, -2) : (lua_isnil(state, -2) ? nullptr : INVALID_POINTER);
			if (f && p != INVALID_POINTER)
			{
				void* ret = nullptr;
				auto ret_type = f->get_type();
				if (ret_type != TypeInfo::get(TypeData, ""))
					ret = ret_type->create(false);

				std::vector<void*> parms;
				parms.resize(f->get_parameters_count());

				for (auto i = 0; i < parms.size(); i++)
				{
					auto type = f->get_parameter(i);
					auto tt = type->get_tag();
					auto tn = std::string(type->get_name());
					auto p = type->create();

					if (lua_istable(state, -3))
					{
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
						else if (tn == "glm::vec<2,float,0>" && tt == TypePointer)
						{
							auto pp = *(void**)p;
							*(vec2*)pp = lua_pull<2>(state);
						}
						else if (tn == "glm::vec<3,float,0>" && tt == TypePointer)
						{
							auto pp = *(void**)p;
							*(vec3*)pp = lua_pull<3>(state);
						}
						else if (tn == "glm::vec<4,float,0>" && tt == TypePointer)
						{
							auto pp = *(void**)p;
							*(vec4*)pp = lua_pull<4>(state);
						}
						else if (tt == TypePointer)
						{
							if (lua_isuserdata(state, -1))
								*(void**)p = lua_touserdata(state, -1);
							else if (lua_isnumber(state, -1))
								*(void**)p = (void*)lua_tointeger(state, -1);
							else if (lua_istable(state, -1))
							{
								lua_pushstring(state, "p");
								lua_gettable(state, -2);
								*(void**)p = lua_isuserdata(state, -1) ? lua_touserdata(state, -1) : nullptr;
								lua_pop(state, 1);
							}
							else
								*(void**)p = nullptr;
						}
						lua_pop(state, 1);
					}

					parms[i] = p;
				}

				lua_pushstring(state, "");
				lua_setglobal(state, "__type__");

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
						lua_getglobal(state, "__type__");
						if (lua_isstring(state, -1))
							tn = lua_tostring(state, -1);
						lua_pop(state, 1);

						lua_newtable(state);
						auto p = *(void**)ret;
						if (p)
						{
							lua_pushstring(state, "p");
							lua_pushlightuserdata(state, p);
							lua_settable(state, -3);
						}

						if (!tn.empty())
							lua_set_object_type(state, tn.c_str());
					}
					else
					{
						if (tn == "int" || tn == "uint")
							lua_pushinteger(state, *(int*)ret);
						else if (tn == "float")
							lua_pushnumber(state, *(float*)ret);
						else if (tn == "glm::vec<2,float,0>")
							lua_push(state, *(vec2*)ret);
						else if (tn == "glm::vec<3,float,0>")
							lua_push(state, *(vec3*)ret);
						else if (tn == "glm::vec<4,float,0>")
							lua_push(state, *(vec4*)ret);
						else
							lua_pushnil(state);
					}
					ret_type->destroy(ret, false);
					return 1;
				}
			}
			return 0;
		}

		int lua_hash(lua_State* state)
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

			lua_pushcfunction(lua_state, lua_flame_call);
			lua_setglobal(lua_state, "flame_call");

			lua_pushcfunction(lua_state, lua_hash);
			lua_setglobal(lua_state, "flame_hash");

			if (!excute(L"setup.lua"))
				fassert(0);

			lua_newtable(lua_state);
			traverse_enums([](Capture& c, EnumInfo* ei) {
				auto state = c.thiz<InstancePrivate>()->lua_state;

				lua_pushstring(state, ei->get_name());

				lua_newtable(state);

				auto count = ei->get_items_count();
				for (auto i = 0; i < count; i++)
				{
					auto item = ei->get_item(i);
					lua_pushstring(state, item->get_name());
					lua_pushinteger(state, item->get_value());
					lua_settable(state, -3);
				}

				lua_settable(state, -3);
			}, Capture().set_thiz(this));
			lua_setglobal(lua_state, "enums");

			lua_newtable(lua_state);
			traverse_udts([](Capture& c, UdtInfo* ui) {
				auto state = c.thiz<InstancePrivate>()->lua_state;
				auto udt_name = std::string(ui->get_name());
				if (!udt_name.ends_with("Private"))
				{
					lua_pushstring(state, udt_name.c_str());

					lua_newtable(state);

					auto count = ui->get_functions_count();
					std::vector<FunctionInfo*> normal_functions;
					std::vector<std::tuple<std::string, FunctionInfo*, FunctionInfo*>> callback_interfaces;
					for (auto i = 0; i < count; i++)
					{
						auto function = ui->get_function(i);
						auto parms_count = function->get_parameters_count();
						for (auto j = 0; j < parms_count; j++)
						{
							auto parameter = function->get_parameter(j);
							if (std::string(parameter->get_name()) == "flame::Capture")
							{
								auto fname = std::string(function->get_name());
								if (fname.starts_with("add_"))
								{
									auto name = fname.substr(4);
									auto function2 = ui->find_function(("remove_" + name).c_str());
									if (function2)
										callback_interfaces.emplace_back(name, function, function2);
								}
								function = nullptr;
								break;
							}
						}
						if (function)
							normal_functions.push_back(function);
					}

					lua_pushstring(state, "functions");
					lua_newtable(state);
					for (auto& f : normal_functions)
					{
						lua_pushstring(state, f->get_name());
						lua_pushlightuserdata(state, f);
						lua_settable(state, -3);
					}
					lua_settable(state, -3);

					lua_pushstring(state, "callback_interfaces");
					lua_newtable(state);
					for (auto& c : callback_interfaces)
					{
						lua_pushstring(state, std::get<0>(c).c_str());

						lua_newtable(state);
						lua_pushstring(state, "add");
						lua_pushlightuserdata(state, std::get<1>(c));
						lua_settable(state, -3);
						lua_pushstring(state, "remove");
						lua_pushlightuserdata(state, std::get<2>(c));
						lua_settable(state, -3);

						lua_settable(state, -3);
					}
					lua_settable(state, -3);

					lua_settable(state, -3);
				}
			}, Capture().set_thiz(this));
			lua_setglobal(lua_state, "udts");

			assert_callback = add_assert_callback([](Capture& c) {
				auto thiz = c.thiz<InstancePrivate>();
				auto state = thiz->lua_state;
				luaL_traceback(state, state, nullptr, 1);
				auto str = std::string(lua_tostring(state, -1));
				if (str != "stack traceback:")
				{
					printf("assertion happened in lua\n%s\nenter debug mode\n", str.c_str());
					lua_getglobal(state, "debug");
					lua_pushstring(state, "debug");
					lua_gettable(state, -2);
					lua_check_result(state, lua_pcall(state, 0, 0, 0));
				}
			}, Capture().set_thiz(this));
		}

		InstancePrivate::~InstancePrivate()
		{
			if (default_instance == this)
				default_instance = nullptr;

			remove_assert_callback(assert_callback);
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
			return lua_check_result(lua_state, luaL_dofile(lua_state, path.string().c_str()));
		}

		void InstancePrivate::push_string(const char* value)
		{
			lua_pushstring(lua_state, value);
		}

		void InstancePrivate::push_pointer(void* p)
		{
			lua_pushlightuserdata(lua_state, p);
		}

		void InstancePrivate::push_object()
		{
			lua_newtable(lua_state);
		}

		void InstancePrivate::pop(uint number)
		{
			lua_pop(lua_state, number);
		}

		void InstancePrivate::get_global(const char* name)
		{
			lua_getglobal(lua_state, name);
		}

		void InstancePrivate::get_member(const char* name, uint idx)
		{
			if (name)
			{
				lua_pushstring(lua_state, name);
				lua_gettable(lua_state, -2);
			}
			else
			{
				lua_pushinteger(lua_state, idx);
				lua_gettable(lua_state, -2);
			}
		}

		void InstancePrivate::set_object_type(const char* type_name)
		{
			lua_set_object_type(lua_state, type_name);
		}

		void InstancePrivate::set_member_name(const char* name)
		{
			lua_pushstring(lua_state, name);
			lua_pushvalue(lua_state, -2);
			lua_settable(lua_state, -4);
			lua_pop(lua_state, 1);
		}

		void InstancePrivate::set_global_name(const char* name)
		{
			lua_setglobal(lua_state, name);
		}

		void InstancePrivate::call(uint parameters_count)
		{
			lua_check_result(lua_state, lua_pcall(lua_state, parameters_count, 0, 0));
		}

		Instance* Instance::get_default()
		{
			return default_instance;
		}

		void Instance::set_default(Instance* instance)
		{
			default_instance = (InstancePrivate*)instance;
		}

		Instance* Instance::create()
		{
			return new InstancePrivate;
		}
	}
}
