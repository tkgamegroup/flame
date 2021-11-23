#pragma once

#ifdef FLAME_FOUNDATION_MODULE
#define FLAME_FOUNDATION_EXPORTS __declspec(dllexport)
template<class T, class U>
struct FlameFoundationTypeSelector
{
	typedef U result;
};
#else
#define FLAME_FOUNDATION_EXPORTS __declspec(dllimport)
template<class T, class U>
struct FlameFoundationTypeSelector
{
	typedef T result;
};
#endif

#define FLAME_FOUNDATION_TYPE(name) struct name; struct name##Private; \
	typedef FlameFoundationTypeSelector<name, name##Private>::result name##T; \
	typedef FlameFoundationTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../serialize.h"

#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

namespace flame
{
	FLAME_FOUNDATION_TYPE(Bitmap);
	FLAME_FOUNDATION_TYPE(NativeWindow);

	enum KeyboardKey
	{
		Keyboard_Backspace,
		Keyboard_Tab,
		Keyboard_Enter,
		Keyboard_Shift,
		Keyboard_Ctrl,
		Keyboard_Alt,
		Keyboard_Pause,
		Keyboard_CapsLock,
		Keyboard_Esc,
		Keyboard_Space,
		Keyboard_PgUp, Keyboard_PgDn,
		Keyboard_End,
		Keyboard_Home,
		Keyboard_Left, Keyboard_Up, Keyboard_Right, Keyboard_Down,
		Keyboard_PrtSc,
		Keyboard_Ins,
		Keyboard_Del,
		Keyboard_0, Keyboard_1, Keyboard_2, Keyboard_3, Keyboard_4, Keyboard_5, Keyboard_6, Keyboard_7, Keyboard_8, Keyboard_9,
		Keyboard_A, Keyboard_B, Keyboard_C, Keyboard_D, Keyboard_E, Keyboard_F, Keyboard_G, Keyboard_H, Keyboard_I, Keyboard_J, Keyboard_K,
		Keyboard_L, Keyboard_M, Keyboard_N, Keyboard_O, Keyboard_P, Keyboard_Q, Keyboard_R, Keyboard_S, Keyboard_T, Keyboard_U, Keyboard_V,
		Keyboard_W, Keyboard_X, Keyboard_Y, Keyboard_Z,
		Keyboard_Numpad0, Keyboard_Numpad1, Keyboard_Numpad2, Keyboard_Numpad3, Keyboard_Numpad4, Keyboard_Numpad5,
		Keyboard_Numpad6, Keyboard_Numpad7, Keyboard_Numpad8, Keyboard_Numpad9,
		Keyboard_Add, Keyboard_Subtract, Keyboard_Multiply, Keyboard_Divide,
		Keyboard_Separator,
		Keyboard_Decimal,
		Keyboard_F1, Keyboard_F2, Keyboard_F3, Keyboard_F4, Keyboard_F5, Keyboard_F6, Keyboard_F7, Keyboard_F8, Keyboard_F9, Keyboard_F10, Keyboard_F11, Keyboard_F12,
		Keyboard_NumLock,
		Keyboard_ScrollLock,

		KeyboardKey_Count
	};

	enum MouseKey
	{
		MouseNull = -1,

		Mouse_Left,
		Mouse_Right,
		Mouse_Middle,

		MouseKeyCount
	};

	enum DragAndDrop
	{
		DragStart,
		DragEnd,
		DragOvering,
		BeingOverStart,
		BeingOvering,
		BeingOverEnd,
		BeenDropped
	};

	struct Guid
	{
		uint d1;
		ushort d2;
		ushort d3;
		uchar d4[8];
	};

	struct Path
	{
		FLAME_FOUNDATION_EXPORTS static std::map<std::wstring, std::filesystem::path> map;

		inline static void add_root(const std::wstring& name, const std::filesystem::path& path)
		{
			map[name] = path;
		}

		inline static std::filesystem::path get(const std::filesystem::path& path)
		{
			auto it = path.begin();
			auto mit = map.find(*it);
			if (mit == map.end())
				return path;
			auto ret = mit->second;
			it++;
			auto eit = path.end();
			while (it != eit)
			{
				ret /= *it;
				it++;
			}
			return ret;
		}
	};

	inline std::vector<std::filesystem::path> get_make_dependencies(const std::filesystem::path& path)
	{
		auto build_path = path.parent_path() / L"build";
		auto dep_path = build_path;
		dep_path /= path.filename();
		dep_path += L".dep";

		std::vector<std::filesystem::path> includes;

		if (!std::filesystem::exists(dep_path) || std::filesystem::last_write_time(dep_path) < std::filesystem::last_write_time(path))
		{
			std::ofstream dep(dep_path);
			dep << path.string() << std::endl;
			includes.push_back(path);
			std::stack<std::filesystem::path> remains;
			remains.push(path);
			while (!remains.empty())
			{
				auto p = remains.top();
				std::ifstream target(p);
				p = p.parent_path();
				remains.pop();
				if (target.good())
				{
					while (!target.eof())
					{
						std::string line;
						std::getline(target, line);
						static auto reg1 = std::regex(R"(#include\s+\"(.*)\")");
						static auto reg2 = std::regex(R"(#include\s+\<(.*)\>)");
						std::smatch res;
						if (std::regex_search(line, res, reg1) || 
							std::regex_search(line, res, reg2))
						{
							auto fn = std::filesystem::path(res[1].str());
							if (!fn.is_absolute())
								fn = p / fn;
							dep << fn.string() << std::endl;
							includes.push_back(fn);
							remains.push(fn);
						}
					}
					target.close();
				}
			}
			dep.close();
		}
		else
		{
			if (!std::filesystem::exists(build_path))
				std::filesystem::create_directories(build_path);

			std::ifstream dep(dep_path);
			while (!dep.eof())
			{
				std::string line;
				std::getline(dep, line);
				if (!line.empty())
					includes.push_back(line);
			}
			dep.close();
		}

		return includes;
	}

	inline bool should_remake(const std::vector<std::filesystem::path>& dependencies, const std::filesystem::path& target)
	{
		if (!std::filesystem::exists(target))
			return true;
		auto lwt = std::filesystem::last_write_time(target);
		for (auto& d : dependencies)
		{
			if (lwt < std::filesystem::last_write_time(d))
				return true;
		}
		return false;
	}

	inline uint64 get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>
			(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	struct ArgPack
	{
		std::string name;
		std::vector<std::string> items;
	};

	struct ArgsPack
	{
		std::map<std::string, ArgPack> args;

		bool has(const std::string& name)
		{
			return args.find(name) != args.end();
		}

		std::string get_item(const std::string& name)
		{
			if (!has(name))
				return "";
			auto& items = args[name].items;
			if (items.size() != 1)
				return "";
			return items[0];
		}

		std::vector<std::string> get_items(const std::string& name)
		{
			if (!has(name))
				return std::vector<std::string>();
			return args[name].items;
		}
	};

	inline ArgsPack parse_args(int argc, char** args)
	{
		ArgsPack ret;
		for (auto i = 1; i < argc; i++)
		{
			auto arg = std::string(args[i]);
			if (arg[0] != '-')
			{
				ArgPack ap;
				ap.name = arg;
				ret.args[arg] = ap;
			}
			else if (arg.size() > 1)
			{
				ArgPack ap;
				ap.name = arg;
				for (++i; i < argc; i++)
				{
					auto arg = std::string(args[i]);
					if (arg[0] == '-')
					{
						i--;
						break;
					}
					else
						ap.items.push_back(arg);
				}
				ret.args[arg] = ap;
			}
		}
		return ret;
	}

	FLAME_FOUNDATION_EXPORTS extern uint frames;
	FLAME_FOUNDATION_EXPORTS extern uint fps;
	FLAME_FOUNDATION_EXPORTS extern float delta_time; // second
	FLAME_FOUNDATION_EXPORTS extern float total_time; // second

	FLAME_FOUNDATION_EXPORTS int run(const std::function<bool()>& callback);

	FLAME_FOUNDATION_EXPORTS void* add_event(const std::function<bool()>& callback, float time = 0.f);
	FLAME_FOUNDATION_EXPORTS void reset_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void remove_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void clear_events();
}
