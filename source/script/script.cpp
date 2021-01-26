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

		void lua_push_vec2(lua_State* state, const vec2& v)
		{
			lua_getglobal(state, "vec2");
			lua_pushnumber(state, v.x);
			lua_pushnumber(state, v.y);
			lua_check_result(state, lua_pcall(state, 2, 1, 0));
		}

		void lua_push_vec3(lua_State* state, const vec3& v)
		{
			lua_getglobal(state, "vec3");
			lua_pushnumber(state, v.x);
			lua_pushnumber(state, v.y);
			lua_pushnumber(state, v.z);
			lua_check_result(state, lua_pcall(state, 3, 1, 0));
		}

		void lua_push_vec4(lua_State* state, const vec4& v)
		{
			lua_getglobal(state, "vec4");
			lua_pushnumber(state, v.x);
			lua_pushnumber(state, v.y);
			lua_pushnumber(state, v.z);
			lua_pushnumber(state, v.w);
			lua_check_result(state, lua_pcall(state, 4, 1, 0));
		}

		vec2 lua_to_vec2(lua_State* state)
		{
			lua_pushstring(state, "push");
			lua_gettable(state, -2);
			lua_check_result(state, lua_pcall(state, 0, 2, 0));
			vec2 ret;
			ret.x = lua_tonumber(state, -2);
			ret.y = lua_tonumber(state, -1);
			lua_pop(state, 2);
			return ret;
		}

		vec3 lua_to_vec3(lua_State* state)
		{
			lua_pushstring(state, "push");
			lua_gettable(state, -2);
			lua_check_result(state, lua_pcall(state, 0, 3, 0));
			vec3 ret;
			ret.x = lua_tonumber(state, -3);
			ret.y = lua_tonumber(state, -2);
			ret.z = lua_tonumber(state, -1);
			lua_pop(state, 3);
			return ret;
		}

		vec4 lua_to_vec4(lua_State* state)
		{
			lua_pushstring(state, "push");
			lua_gettable(state, -2);
			lua_check_result(state, lua_pcall(state, 0, 4, 0));
			vec4 ret;
			ret.x = lua_tonumber(state, -4);
			ret.y = lua_tonumber(state, -3);
			ret.z = lua_tonumber(state, -2);
			ret.w = lua_tonumber(state, -1);
			lua_pop(state, 4);
			return ret;
		}

		thread_local mat4 matrices[100];

		int lua_flame_transform(lua_State* state)
		{
			auto mat_id = lua_isinteger(state, -2) ? lua_tointeger(state, -2) : -1;
			auto v = lua_to_vec4(state);
			if (mat_id != -1)
			{
				lua_push_vec4(state, matrices[mat_id] * v);
				return 1;
			}
			return 0;
		}

		int lua_flame_perspective(lua_State* state)
		{
			auto mat_id = lua_isinteger(state, -5) ? lua_tointeger(state, -5) : -1;
			auto fovy = lua_isnumber(state, -4) ? lua_tonumber(state, -4) : 0.f;
			auto aspect = lua_isnumber(state, -3) ? lua_tonumber(state, -3) : 0.f;
			auto zNear = lua_isnumber(state, -2) ? lua_tonumber(state, -2) : 0.f;
			auto zFar = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : 0.f;
			if (mat_id != -1)
			{
				matrices[mat_id] = perspective(radians(fovy), aspect, zNear, zFar);
				matrices[mat_id][1][1] *= -1.f;
			}
			return 0;
		}

		int lua_flame_call(lua_State* state)
		{
			auto o = lua_isuserdata(state, -2) ? lua_touserdata(state, -2) : nullptr;
			auto f = lua_isuserdata(state, -1) ? (FunctionInfo*)lua_touserdata(state, -1) : nullptr;
			if (f && o)
			{
				char parms[4 * sizeof(void*)];
				auto parms_count = f->get_parameters_count();
				auto p = parms;
				std::vector<std::unique_ptr<std::string>> temp_strs;
				std::vector<std::unique_ptr<std::wstring>> temp_wstrs;
				std::vector<std::unique_ptr<char>> temp_datas;
				for (auto i = 0; i < parms_count; i++)
				{
					auto type = f->get_parameter(i);
					auto tag = type->get_tag();
					auto basic = type->get_basic();

					if (lua_istable(state, -3))
					{
						lua_pushinteger(state, i + 1);
						lua_gettable(state, -4);

						switch (type->get_tag())
						{
						case TypeData:
							switch (basic)
							{
							case BooleanType:
								*(bool*)p = lua_isboolean(state, -1) ? lua_toboolean(state, -1) : true;
								p += sizeof(bool);
								break;
							case IntegerType:
								*(int*)p = lua_isinteger(state, -1) ? lua_tointeger(state, -1) : -1;
								p += sizeof(int);
								break;
							case FloatingType:
								*(float*)p = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : -1.f;
								p += sizeof(float);
								break;
							}
							break;
						case TypePointer:
							auto pointed_type = type->get_pointed_type();
							auto basic = pointed_type ? pointed_type->get_basic() : ElseType;
							auto vec_size = pointed_type ? pointed_type->get_vec_size() : 1;
							if (vec_size == 1)
							{
								switch (basic)
								{
								case CharType:
								{
									auto t = type->create();
									auto str = new std::string(lua_isstring(state, -1) ? lua_tostring(state, -1) : "");
									*(void**)p = (char*)str->c_str();
									temp_strs.emplace_back(str);
								}
									break;
								case WideCharType:
								{
									auto str = new std::wstring(s2w(lua_isstring(state, -1) ? lua_tostring(state, -1) : ""));
									*(void**)p = (wchar_t*)str->c_str();
									temp_wstrs.emplace_back(str);
								}
									break;
								default:
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
							}
							else
							{
								auto d = new char[pointed_type->get_size()];
								switch (basic)
								{
								case FloatingType:
									switch (vec_size)
									{
									case 2:
										*(vec2*)d = lua_to_vec2(state);
										break;
									case 3:
										*(vec3*)d = lua_to_vec3(state);
										break;
									case 4:
										*(vec4*)d = lua_to_vec4(state);
										break;
									}
									break;
								case CharType:
									switch (vec_size)
									{
									case 4:
										*(cvec4*)d = lua_to_vec4(state);
										break;
									}
									break;
								}
								*(void**)p = d;
								temp_datas.emplace_back(d);
							}

							p += sizeof(void*);
							break;
						}
						lua_pop(state, 1);
					}
				}

				void* ret = nullptr;
				auto ret_type = f->get_type();
				if (ret_type != TypeInfo::get(TypeData, ""))
					ret = ret_type->create(false);

				f->call(o, ret, parms);

				if (ret)
				{
					auto pushed_number = 1;
					if (ret_type->get_tag() == TypePointer)
						lua_pushlightuserdata(state, *(void**)ret);
					else
					{
						auto basic = ret_type->get_basic();
						auto vec_size = ret_type->get_vec_size();
						switch (ret_type->get_col_size())
						{
						case 1:
							switch (vec_size)
							{
							case 1:
								switch (basic)
								{
								case IntegerType:
									lua_pushinteger(state, *(int*)ret);
									break;
								case FloatingType:
									lua_pushnumber(state, *(float*)ret);
									break;
								}
								break;
							case 2:
								switch (basic)
								{
								case FloatingType:
									lua_push_vec2(state, *(vec2*)ret);
									break;
								}
								break;
							case 3:
								switch (basic)
								{
								case FloatingType:
									lua_push_vec3(state, *(vec3*)ret);
									break;
								}
								break;
							case 4:
								switch (basic)
								{
								case FloatingType:
									lua_push_vec4(state, *(vec4*)ret);
									break;
								}
								break;
							}
							break;
						case 4:
							switch (vec_size)
							{
							case 4:
								switch (basic)
								{
								case FloatingType:
									lua_getglobal(state, "__mat_id__");
									auto mat_id = lua_isinteger(state, -1) ? lua_tointeger(state, -1) : -1;
									lua_pop(state, 1);
									if (mat_id != -1)
										memcpy(&matrices[mat_id], ret, sizeof(mat4));
									pushed_number = 0;
									break;
								}
								break;
							}
							break;
						}
					}
					ret_type->destroy(ret, false);
					return pushed_number;
				}
			}
			return 0;
		}

		int lua_flame_hash(lua_State* state)
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

			lua_pushcfunction(lua_state, lua_flame_hash);
			lua_setglobal(lua_state, "flame_hash");

			lua_pushcfunction(lua_state, lua_flame_transform);
			lua_setglobal(lua_state, "flame_transform");

			lua_pushcfunction(lua_state, lua_flame_perspective);
			lua_setglobal(lua_state, "flame_perspective");

			if (!excute_file(L"setup.lua"))
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
					std::vector<std::pair<FunctionInfo*, std::string>> type_needed_functions;
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
						{
							auto ret = function->get_type();
							if (ret->get_tag() == TypePointer)
							{
								auto type = std::string(ret->get_name());
								if (type != "void")
								{
									type_needed_functions.emplace_back(function, type);
									function = nullptr;
								}
							}
						}
						if (function)
							normal_functions.push_back(function);
					}

					lua_pushstring(state, "normal_functions");
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

					lua_pushstring(state, "type_needed_functions");
					lua_newtable(state);
					for (auto& f : type_needed_functions)
					{
						lua_pushstring(state, f.first->get_name());

						lua_newtable(state);
						lua_pushstring(state, "func");
						lua_pushlightuserdata(state, f.first);
						lua_settable(state, -3);
						lua_pushstring(state, "type");
						lua_pushstring(state, f.second.c_str());
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

		void InstancePrivate::push_int(int i)
		{
			lua_pushinteger(lua_state, i);
		}

		void InstancePrivate::push_float(float f)
		{
			lua_pushnumber(lua_state, f);
		}

		void InstancePrivate::push_vec2(const vec2& v)
		{
			lua_push_vec2(lua_state, v);
		}

		void InstancePrivate::push_vec3(const vec3& v)
		{
			lua_push_vec3(lua_state, v);
		}

		void InstancePrivate::push_vec4(const vec4& v)
		{
			lua_push_vec4(lua_state, v);
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

		void InstancePrivate::set_object_type(const char* type_name, void* p)
		{
			if (p != INVALID_POINTER)
			{
				lua_pushstring(lua_state, "p");
				lua_pushlightuserdata(lua_state, p);
				lua_settable(lua_state, -3);
			}
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

		bool InstancePrivate::excute(const char* str)
		{
			return lua_check_result(lua_state, luaL_dostring(lua_state, str));
		}

		bool InstancePrivate::excute_file(const wchar_t* filename)
		{
			auto path = std::filesystem::path(filename);
			if (!std::filesystem::exists(path))
			{
				auto engine_path = getenv("FLAME_PATH");
				if (engine_path)
					path = std::filesystem::path(engine_path) / "assets" / path;
			}
			return lua_check_result(lua_state, luaL_dofile(lua_state, path.string().c_str()));
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
