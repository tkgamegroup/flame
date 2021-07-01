#include "../foundation/typeinfo.h"
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

		vec2 lua_to_vec2(lua_State* state, int idx)
		{
			lua_pushvalue(state, idx);
			lua_pushstring(state, "push");
			lua_gettable(state, -2);
			lua_check_result(state, lua_pcall(state, 0, 2, 0));
			vec2 ret;
			ret.x = lua_tonumber(state, -2);
			ret.y = lua_tonumber(state, -1);
			lua_pop(state, 3);
			return ret;
		}

		vec3 lua_to_vec3(lua_State* state, int idx)
		{
			lua_pushvalue(state, idx);
			lua_pushstring(state, "push");
			lua_gettable(state, -2);
			lua_check_result(state, lua_pcall(state, 0, 3, 0));
			vec3 ret;
			ret.x = lua_tonumber(state, -3);
			ret.y = lua_tonumber(state, -2);
			ret.z = lua_tonumber(state, -1);
			lua_pop(state, 4);
			return ret;
		}

		vec4 lua_to_vec4(lua_State* state, int idx)
		{
			lua_pushvalue(state, idx);
			lua_pushstring(state, "push");
			lua_gettable(state, -2);
			lua_check_result(state, lua_pcall(state, 0, 4, 0));
			vec4 ret;
			ret.x = lua_tonumber(state, -4);
			ret.y = lua_tonumber(state, -3);
			ret.z = lua_tonumber(state, -2);
			ret.w = lua_tonumber(state, -1);
			lua_pop(state, 5);
			return ret;
		}

		int lua_flame_malloc(lua_State* state)
		{
			if (lua_isinteger(state, -1))
			{
				lua_pushlightuserdata(state, malloc(lua_tointeger(state, -1)));
				return 1;
			}
			return 0;
		}

		int lua_flame_free(lua_State* state)
		{
			if (lua_isuserdata(state, -1))
				free(lua_touserdata(state, -1));
			return 0;
		}

		int lua_flame_hash(lua_State* state)
		{
			if (lua_isstring(state, -1))
			{
				auto hash = (uint)std::hash<std::string>()(lua_tostring(state, -1));
				lua_pushinteger(state, hash);
				return 1;
			}
			return 0;
		}

		thread_local mat4 matrices[100];

		int lua_flame_transform(lua_State* state)
		{
			auto mat_id = lua_isinteger(state, -2) ? lua_tointeger(state, -2) : -1;
			auto v = lua_to_vec4(state, -1);
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

		int lua_flame_get_fps(lua_State* state)
		{
			lua_pushinteger(state, looper().get_fps());
			return 1;
		}

		int lua_flame_get(lua_State* state)
		{
			auto obj = lua_isuserdata(state, -6) ? lua_touserdata(state, -6) : nullptr;
			auto off = lua_isinteger(state, -5) ? lua_tointeger(state, -5) : 0;
			auto tag = lua_isinteger(state, -4) ? (TypeTag)lua_tointeger(state, -4) : TypeData;
			auto basic = lua_isinteger(state, -3) ? (BasicType)lua_tointeger(state, -3) : VoidType;
			auto vec_size = lua_isinteger(state, -2) ? lua_tointeger(state, -2) : 0;
			auto col_size = lua_isinteger(state, -1) ? lua_tointeger(state, -1) : 0;
			if (obj)
			{
				switch (tag)
				{
				case TypeData:
					switch (vec_size)
					{
					case 1:
						switch (basic)
						{
						case IntegerType:
						{
							int ret;
							ret = *(int*)((char*)obj + off);
							lua_pushinteger(state, ret);
							return 1;
						}
						case FloatingType:
						{
							float ret;
							ret = *(float*)((char*)obj + off);
							lua_pushnumber(state, ret);
							return 1;
						}
						}
						break;
					case 2:
						switch (basic)
						{
						case FloatingType:
						{
							vec2 ret;
							ret = *(vec2*)((char*)obj + off);
							lua_push_vec2(state, ret);
							return 1;
						}
						}
						break;
					case 3:
						switch (basic)
						{
						case FloatingType:
						{
							vec3 ret;
							ret = *(vec3*)((char*)obj + off);
							lua_push_vec3(state, ret);
							return 1;
						}
						}
						break;
					case 4:
						switch (basic)
						{
						case FloatingType:
						{
							vec4 ret;
							ret = *(vec4*)((char*)obj + off);
							lua_push_vec4(state, ret);
							return 1;
						}
						}
						break;
					}
					break;
				case TypePointer:
					switch (basic)
					{
					case CharType:
					{
						char** pstr = (char**)((char*)obj + off);
						lua_pushstring(state, *pstr ? *pstr : "");
					}
						break;
					case WideCharType:
					{
						wchar_t** pstr = (wchar_t**)((char*)obj + off);
						lua_pushstring(state, w2s(*pstr ? *pstr : L"").c_str());
					}
						break;
					default:
					{
						void* ret;
						ret = *(void**)((char*)obj + off);
						if (ret)
							lua_pushlightuserdata(state, ret);
						else
							lua_pushnil(state);
					}
						break;
					}
					return 1;
				}
			}
			return 0;
		}

		int lua_flame_set(lua_State* state)
		{
			auto obj = lua_isuserdata(state, -7) ? lua_touserdata(state, -7) : nullptr;
			auto off = lua_isinteger(state, -6) ? lua_tointeger(state, -6) : 0;
			auto tag = lua_isinteger(state, -5) ? (TypeTag)lua_tointeger(state, -5) : TypeData;
			auto basic = lua_isinteger(state, -4) ? (BasicType)lua_tointeger(state, -4) : VoidType;
			auto vec_size = lua_isinteger(state, -3) ? lua_tointeger(state, -3) : 0;
			auto col_size = lua_isinteger(state, -2) ? lua_tointeger(state, -2) : 0;
			if (obj)
			{
				switch (tag)
				{
				case TypeData:
					switch (vec_size)
					{
					case 1:
						switch (basic)
						{
						case IntegerType:
							*(int*)((char*)obj + off) = lua_isinteger(state, -1) ? lua_tointeger(state, -1) : 0;
							break;
						case FloatingType:
							*(float*)((char*)obj + off) = lua_isnumber(state, -1) ? lua_tonumber(state, -1) : 0;
							break;
						}
						break;
					case 2:
						switch (basic)
						{
						case FloatingType:
							*(vec2*)((char*)obj + off) = lua_to_vec2(state, -1);
							break;
						}
						break;
					case 3:
						switch (basic)
						{
						case FloatingType:
							*(vec3*)((char*)obj + off) = lua_to_vec3(state, -1);
							break;
						}
						break;
					case 4:
						switch (basic)
						{
						case FloatingType:
							*(vec4*)((char*)obj + off) = lua_to_vec4(state, -1);
							break;
						}
						break;
					}
					break;
				}
			}
			return 0;
		}

		int lua_flame_load_file(lua_State* state)
		{
			if (lua_isstring(state, -1))
			{
				std::filesystem::path fn = lua_tostring(state, -1);
				if (std::filesystem::exists(fn))
				{
					auto str = get_file_content(fn);
					lua_pushstring(state, str.c_str());
					return 1;
				}
			}
			return 0;
		}

		int lua_flame_save_file(lua_State* state)
		{
			if (lua_isstring(state, -1) && lua_isstring(state, -2))
			{
				std::filesystem::path fn = lua_tostring(state, -2);
				std::string content = lua_tostring(state, -1);
				std::ofstream file(fn, std::ios::binary);
				file.write(content.c_str(), content.size());
				file.close();
			}
			return 0;
		}

		int lua_flame_get_directory_files(lua_State* state)
		{
			if (lua_isstring(state, -1))
			{
				std::filesystem::path fn = lua_tostring(state, -1);
				auto ok = std::filesystem::exists(fn);
				if (!ok)
				{
					auto engine_path = getenv("FLAME_PATH");
					if (engine_path)
						fn = std::filesystem::path(engine_path) / "assets" / fn;
					ok = std::filesystem::exists(fn);
				}
				lua_newtable(state);
				auto idx = 1;
				for (auto& p : std::filesystem::directory_iterator(fn))
				{
					if (!p.is_directory())
					{
						lua_pushinteger(state, idx++);
						lua_pushstring(state, p.path().filename().string().c_str());
						lua_settable(state, -3);
					}
				}
				return 1;
			}
			return 0;
		}

		int lua_flame_call(lua_State* state)
		{
			auto o = lua_isuserdata(state, -3) ? lua_touserdata(state, -3) : nullptr;
			auto f = lua_isuserdata(state, -2) ? (FunctionInfo*)lua_touserdata(state, -2) : nullptr;
			if (f)
			{
				auto parms_cnt = f->get_parameters_count();
				std::vector<void*> ps;
				ps.resize(parms_cnt);
				static char buf[1024 * 1024];
				auto p = buf;
				for (auto i = 0; i < parms_cnt; i++)
				{
					auto type = f->get_parameter(i);
					auto tag = type->get_tag();
					auto basic = type->get_basic();
					auto vec_size = type->get_vec_size();

					ps[i] = p;
					if (lua_istable(state, -1))
					{
						lua_pushinteger(state, i + 1);
						lua_gettable(state, -2);

						switch (type->get_tag())
						{
						case TypeEnumSingle:
						case TypeEnumMulti:
							*(int*)p = lua_isinteger(state, -1) ? lua_tointeger(state, -1) : -1;
							p += sizeof(int);
							break;
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
							if (lua_isuserdata(state, -1))
							{
								*(void**)p = lua_touserdata(state, -1);
								p += sizeof(void*);
							}
							else if (lua_isnil(state, -1))
							{
								*(void**)p = nullptr;
								p += sizeof(void*);
							}
							else
							{
								switch (vec_size)
								{
								case 1:
									switch (basic)
									{
									case CharType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										if (lua_isstring(state, -1))
										{
											auto str = lua_tostring(state, -1);
											auto len = strlen(str);
											strncpy(p, str, len);
											p[len] = 0;
											p += len + 1;
										}
										else
										{
											*p = 0;
											p++;
										}
									}
										break;
									case WideCharType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										if (lua_isstring(state, -1))
										{
											auto str = s2w(lua_tostring(state, -1));
											wcsncpy((wchar_t*)p, str.c_str(), str.size());
											((wchar_t*)p)[str.size()] = 0;
											p += (str.size() + 1) * sizeof(wchar_t);
										}
										else
										{
											*(wchar_t*)p = 0;
											p += sizeof(wchar_t);
										}
									}
										break;
									default:
										if (lua_isnumber(state, -1))
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
										p += sizeof(void*);
									}
									break;
								case 2:
									switch (basic)
									{
									case IntegerType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(ivec2*)p = ivec2(lua_to_vec2(state, -1));
										p += sizeof(ivec2);
									}
										break;
									case FloatingType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(vec2*)p = vec2(lua_to_vec2(state, -1));
										p += sizeof(vec2);
									}
										break;
									case CharType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(cvec2*)p = cvec2(lua_to_vec2(state, -1));
										p += sizeof(cvec2);
									}
										break;
									}
									break;
								case 3:
									switch (basic)
									{
									case IntegerType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(ivec3*)p = ivec3(lua_to_vec3(state, -1));
										p += sizeof(ivec3);
									}
										break;
									case FloatingType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(vec3*)p = vec3(lua_to_vec3(state, -1));
										p += sizeof(vec3);
									}
										break;
									case CharType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(cvec3*)p = cvec3(lua_to_vec3(state, -1));
										p += sizeof(cvec3);
									}
										break;
									}
									break;
								case 4:
									switch (basic)
									{
									case IntegerType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(ivec4*)p = ivec4(lua_to_vec4(state, -1));
										p += sizeof(ivec4);
									}
										break;
									case FloatingType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(vec4*)p = vec4(lua_to_vec4(state, -1));
										p += sizeof(vec4);
									}
										break;
									case CharType:
									{
										*(void**)p = (char*)p + sizeof(void*);
										p += sizeof(void*);
										*(cvec4*)p = cvec4(lua_to_vec4(state, -1));
										p += sizeof(cvec4);
									}
										break;
									}
									break;
								}
							}

							break;
						}
						lua_pop(state, 1);
					}
				}

				void* ret = nullptr;
				auto ret_type = f->get_type();
				if (ret_type != TypeInfo::get(TypeData, ""))
					ret = ret_type->create(false);

				f->call(o, ret, ps.data());

				if (ret)
				{
					auto pushed_number = 0;
					auto basic = ret_type->get_basic();
					auto vec_size = ret_type->get_vec_size();
					auto col_size = ret_type->get_col_size();
					switch (ret_type->get_tag())
					{
					case TypeData:
					{
						switch (col_size)
						{
						case 1:
							switch (vec_size)
							{
							case 1:
								switch (basic)
								{
								case BooleanType:
									lua_pushboolean(state, *(bool*)ret);
									pushed_number = 1;
									break;
								case IntegerType:
									lua_pushinteger(state, *(int*)ret);
									pushed_number = 1;
									break;
								case FloatingType:
									lua_pushnumber(state, *(float*)ret);
									pushed_number = 1;
									break;
								}
								break;
							case 2:
								switch (basic)
								{
								case IntegerType:
									lua_push_vec2(state, *(ivec2*)ret);
									pushed_number = 1;
									break;
								case FloatingType:
									lua_push_vec2(state, *(vec2*)ret);
									pushed_number = 1;
									break;
								case CharType:
									lua_push_vec2(state, *(cvec2*)ret);
									pushed_number = 1;
									break;
								}
								break;
							case 3:
								switch (basic)
								{
								case IntegerType:
									lua_push_vec3(state, *(ivec3*)ret);
									pushed_number = 1;
									break;
								case FloatingType:
									lua_push_vec3(state, *(vec3*)ret);
									pushed_number = 1;
									break;
								case CharType:
									lua_push_vec3(state, *(cvec3*)ret);
									pushed_number = 1;
									break;
								}
								break;
							case 4:
								switch (basic)
								{
								case IntegerType:
									lua_push_vec4(state, *(ivec4*)ret);
									pushed_number = 1;
									break;
								case FloatingType:
									lua_push_vec4(state, *(vec4*)ret);
									pushed_number = 1;
									break;
								case CharType:
									lua_push_vec4(state, *(cvec4*)ret);
									pushed_number = 1;
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
									break;
								}
								break;
							}
							break;
						}
					}
						break;
					case TypePointer:
					{
						auto pointer = *(void**)ret;
						if (pointer)
						{
							switch (ret_type->get_basic())
							{
							case CharType:
								lua_pushstring(state, pointer ? (char*)pointer : "");
								pushed_number = 1;
								break;
							case WideCharType:
								lua_pushstring(state, pointer ? w2s((wchar_t*)pointer).c_str() : "");
								pushed_number = 1;
								break;
							default:
								lua_pushlightuserdata(state, pointer);
								pushed_number = 1;
								break;
							}
						}
						else
						{
							lua_pushnil(state);
							pushed_number = 1;
						}
					}
						break;
					}
					ret_type->destroy(ret, false);
					return pushed_number;
				}
			}
			return 0;
		}

		InstancePrivate::InstancePrivate()
		{
			lua_state = luaL_newstate();
			luaL_openlibs(lua_state);

			lua_pushcfunction(lua_state, lua_flame_hash);
			lua_setglobal(lua_state, "flame_hash");

			lua_pushcfunction(lua_state, lua_flame_malloc);
			lua_setglobal(lua_state, "flame_malloc");

			lua_pushcfunction(lua_state, lua_flame_free);
			lua_setglobal(lua_state, "flame_free");

			lua_pushcfunction(lua_state, lua_flame_transform);
			lua_setglobal(lua_state, "flame_transform");

			lua_pushcfunction(lua_state, lua_flame_perspective);
			lua_setglobal(lua_state, "flame_perspective");

			lua_pushcfunction(lua_state, lua_flame_get_fps);
			lua_setglobal(lua_state, "flame_get_fps");

			lua_pushcfunction(lua_state, lua_flame_get);
			lua_setglobal(lua_state, "flame_get");

			lua_pushcfunction(lua_state, lua_flame_set);
			lua_setglobal(lua_state, "flame_set");

			lua_pushcfunction(lua_state, lua_flame_call);
			lua_setglobal(lua_state, "flame_call");

			lua_pushcfunction(lua_state, lua_flame_load_file);
			lua_setglobal(lua_state, "flame_load_file");

			lua_pushcfunction(lua_state, lua_flame_save_file);
			lua_setglobal(lua_state, "flame_save_file");

			lua_pushcfunction(lua_state, lua_flame_get_directory_files);
			lua_setglobal(lua_state, "flame_get_directory_files");
			
			if (!excute_file(L"setup.lua"))
				fassert(0);

			lua_newtable(lua_state);
			auto types = get_types();
			for (auto ti : types)
			{
				auto is_object_type = false;
				if (ti->get_tag() == TypePointer)
				{
					auto basic = ti->get_basic();
					if (basic != VoidType && basic != CharType && basic != WideCharType)
						is_object_type = true;
				}

				lua_pushinteger(lua_state, ti->get_hash());

				lua_newtable(lua_state);

				lua_pushstring(lua_state, "tag");
				lua_pushinteger(lua_state, ti->get_tag());
				lua_settable(lua_state, -3);
				lua_pushstring(lua_state, "name");
				lua_pushstring(lua_state, ti->get_name());
				lua_settable(lua_state, -3);
				lua_pushstring(lua_state, "basic");
				lua_pushinteger(lua_state, ti->get_basic());
				lua_settable(lua_state, -3);
				lua_pushstring(lua_state, "vec_size");
				lua_pushinteger(lua_state, ti->get_vec_size());
				lua_settable(lua_state, -3);
				lua_pushstring(lua_state, "col_size");
				lua_pushinteger(lua_state, ti->get_col_size());
				lua_settable(lua_state, -3);
				lua_pushstring(lua_state, "is_object_type");
				lua_pushboolean(lua_state, is_object_type);
				lua_settable(lua_state, -3);

				lua_settable(lua_state, -3);
			}
			lua_setglobal(lua_state, "types");

			lua_newtable(lua_state);
			auto enums = get_enums();
			for (auto ei : enums)
			{
				lua_pushstring(lua_state, ei->get_name());

				lua_newtable(lua_state);

				auto count = ei->get_items_count();
				for (auto i = 0; i < count; i++)
				{
					auto item = ei->get_item(i);
					lua_pushstring(lua_state, item->get_name());
					lua_pushinteger(lua_state, item->get_value());
					lua_settable(lua_state, -3);
				}

				lua_settable(lua_state, -3);
			}
			lua_setglobal(lua_state, "enums");

			lua_newtable(lua_state);
			auto udts = get_udts();
			for (auto ui : udts)
			{
				auto udt_name = std::string(ui->get_name());
				if (udt_name.ends_with("Private"))
					continue;

				lua_pushstring(lua_state, udt_name.c_str());

				lua_newtable(lua_state);

				lua_pushstring(lua_state, "base");
				lua_pushstring(lua_state, ui->get_base_name());
				lua_settable(lua_state, -3);

				lua_pushstring(lua_state, "variables");
				lua_newtable(lua_state);
				auto var_cnt = ui->get_variables_count();
				for (auto i = 0; i < var_cnt; i++)
				{
					auto variable = ui->get_variable(i);
					auto type = variable->get_type();

					lua_pushstring(lua_state, variable->get_name());

					lua_newtable(lua_state);
					lua_pushstring(lua_state, "offset");
					lua_pushinteger(lua_state, variable->get_offset());
					lua_settable(lua_state, -3);
					lua_pushstring(lua_state, "type_name");
					lua_pushinteger(lua_state, type->get_hash());
					lua_settable(lua_state, -3);

					lua_settable(lua_state, -3);
				}
				lua_settable(lua_state, -3);

				auto fun_cnt = ui->get_functions_count();
				std::vector<FunctionInfo*> functions;
				std::vector<FunctionInfo*> callbacks;
				std::vector<std::tuple<std::string, FunctionInfo*, FunctionInfo*>> listeners;
				for (auto i = 0; i < fun_cnt; i++)
				{
					auto function = ui->get_function(i);
					auto parms_cnt = function->get_parameters_count();
					for (auto j = 0; j < parms_cnt; j++)
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
									listeners.emplace_back(name, function, function2);
							}
							else
								callbacks.push_back(function);
							function = nullptr;
							break;
						}
					}
					if (function)
						functions.push_back(function);
				}

				lua_pushstring(lua_state, "functions");
				lua_newtable(lua_state);
				for (auto& f : functions)
				{
					lua_pushstring(lua_state, f->get_name());

					lua_newtable(lua_state);
					lua_pushstring(lua_state, "f");
					lua_pushlightuserdata(lua_state, f);
					lua_settable(lua_state, -3);
					lua_pushstring(lua_state, "ret_type_name");
					lua_pushinteger(lua_state, f->get_type()->get_hash());
					lua_settable(lua_state, -3);
					lua_pushstring(lua_state, "static");
					lua_pushboolean(lua_state, (bool)f->get_rva());
					lua_settable(lua_state, -3);
					lua_pushstring(lua_state, "index");
					lua_pushinteger(lua_state, f->get_index());
					lua_settable(lua_state, -3);

					lua_settable(lua_state, -3);
				}
				lua_settable(lua_state, -3);

				lua_pushstring(lua_state, "callbacks");
				lua_newtable(lua_state);
				for (auto c : callbacks)
				{
					lua_pushstring(lua_state, c->get_name());
					lua_pushlightuserdata(lua_state, c);
					lua_settable(lua_state, -3);
				}
				lua_settable(lua_state, -3);

				lua_pushstring(lua_state, "listeners");
				lua_newtable(lua_state);
				for (auto& l : listeners)
				{
					lua_pushstring(lua_state, std::get<0>(l).c_str());

					lua_newtable(lua_state);
					lua_pushstring(lua_state, "add");
					lua_pushlightuserdata(lua_state, std::get<1>(l));
					lua_settable(lua_state, -3);
					lua_pushstring(lua_state, "remove");
					lua_pushlightuserdata(lua_state, std::get<2>(l));
					lua_settable(lua_state, -3);

					lua_settable(lua_state, -3);
				}
				lua_settable(lua_state, -3);

				lua_settable(lua_state, -3);
			}
			lua_setglobal(lua_state, "udts");

			assert_callback = add_assert_callback([](Capture& c) {
				auto thiz = c.thiz<InstancePrivate>();
				auto state = thiz->lua_state;
				luaL_traceback(state, state, nullptr, 1);
				auto str = std::string(lua_tostring(state, -1));
				if (str != "stack traceback:")
				{
					printf("assertion happened in lua\n%s\nenter debug mode (cont to exit)\n", str.c_str());
					lua_getglobal(state, "debug");
					lua_pushstring(state, "debug");
					lua_gettable(state, -2);
					lua_check_result(state, lua_pcall(state, 0, 0, 0));
				}
			}, Capture().set_thiz(this));

			if (!excute_file(L"setup2.lua"))
				fassert(0);
		}

		InstancePrivate::~InstancePrivate()
		{
			if (default_instance == this)
				default_instance = nullptr;

			remove_assert_callback(assert_callback);
		}

		int InstancePrivate::to_int(int idx)
		{
			return lua_tointeger(lua_state, idx);
		}

		uint InstancePrivate::to_uint(int idx)
		{
			return lua_tointeger(lua_state, idx);
		}

		void InstancePrivate::push_bool(bool b)
		{
			lua_pushboolean(lua_state, b);
		}

		void InstancePrivate::push_int(int i)
		{
			lua_pushinteger(lua_state, i);
		}

		void InstancePrivate::push_uint(uint i)
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

		void InstancePrivate::call(uint parms_num)
		{
			lua_check_result(lua_state, lua_pcall(lua_state, parms_num, 0, 0));
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
				if (!std::filesystem::exists(path))
				{
					wprintf(L"script not found: %s\n", path.c_str());
					return true;
				}
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
