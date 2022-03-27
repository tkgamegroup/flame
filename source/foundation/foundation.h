#pragma once

#include "../serialize.h"

#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

#ifdef FLAME_FOUNDATION_MODULE

#define FLAME_FOUNDATION_API __declspec(dllexport)

#define FLAME_FOUNDATION_TYPE(name) FLAME_TYPE_PRIVATE(name)

#else

#define FLAME_FOUNDATION_API __declspec(dllimport)

#define FLAME_FOUNDATION_TYPE(name) FLAME_TYPE(name)

#endif

namespace flame
{
	FLAME_FOUNDATION_TYPE(Bitmap);
	FLAME_FOUNDATION_TYPE(NativeWindow);

	struct TypeInfo;
	struct VariableInfo;
	struct EnumItemInfo;
	struct EnumInfo;
	struct FunctionInfo;
	struct UdtInfo;
	struct TypeInfoDataBase;

	enum MouseButton
	{
		Mouse_Left,
		Mouse_Right,
		Mouse_Middle,

		MouseButton_Count
	};

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

	inline std::vector<std::string> format_defines(const std::string& str)
	{
		std::vector<std::string> ret;
		auto sp = SUS::split(str, ';');
		for (auto& s : sp)
		{
			SUS::trim(s);
			if (!s.empty())
				ret.push_back(s);
		}
		std::sort(ret.begin(), ret.end());
		return ret;
	}

	struct Path
	{
		FLAME_FOUNDATION_API static std::map<std::wstring, std::filesystem::path> roots;

		inline static void set_root(const std::wstring& name, const std::filesystem::path& path)
		{
			roots[name] = path;
		}

		inline static std::filesystem::path get(const std::filesystem::path& path)
		{
			if (path.is_absolute())
				return path;
			auto it = path.begin();
			auto it2 = roots.find(*it);
			if (it2 == roots.end())
				return L"";
			auto ret = it2->second;
			it++;
			auto eit = path.end();
			while (it != eit)
			{
				ret /= *it;
				it++;
			}
			ret.make_preferred();
			return ret;
		}

		inline static std::filesystem::path reverse(const std::filesystem::path& _path)
		{
			auto path = _path;
			path.make_preferred();
			if (!path.is_absolute())
				return path;
			auto str1 = path.wstring();
			for (auto& r : roots)
			{
				auto str2 = r.second.wstring();
				if (str1.compare(0, str2.size(), str2) == 0)
					return r.first / std::filesystem::path(str1.substr(str2.size() + 1));
			}
			return path;
		}

		inline static bool cat_if_exists(const std::filesystem::path& dir, std::filesystem::path& t)
		{
			if (t.is_absolute())
				return false;
			auto p = dir / t;
			if (std::filesystem::exists(p))
			{
				t = p;
				return true;
			}
			return false;
		}
	};

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

	template<typename F>
	struct Listeners
	{
		std::list<std::pair<std::function<F>, uint>> list;

		void* add(const std::function<F>& callback, uint h = 0, bool back = true)
		{
			if (back)
				return &list.emplace_back(callback, h);
			return &list.emplace_front(callback, h);
		}

		void remove(void* lis)
		{
			std::erase_if(list, [&](const auto& i) {
				return &i == lis;
			});
		}

		void remove(uint h)
		{
			std::erase_if(list, [&](const auto& i) {
				return i.second == h;
			});
		}

		bool exist(uint h)
		{
			return std::find_if(list.begin(), list.end(), [&](const auto& i) {
				return i.second == h;
			}) != list.end();
		}
	};

	struct AssetManagemant
	{
		struct Asset
		{
			uint type;
			void* obj;

			uint ref = 0;
		};

		FLAME_FOUNDATION_API static std::map<std::filesystem::path, Asset> assets;

		inline static Asset* find(const std::filesystem::path& path)
		{
			auto it = assets.find(path);
			if (it == assets.end())
				return nullptr;
			return &it->second;
		}

		FLAME_FOUNDATION_API static Asset& get_asset(const std::filesystem::path& path);
		FLAME_FOUNDATION_API static void release_asset(const std::filesystem::path& path);
	};

	FLAME_FOUNDATION_API extern uint frames;
	FLAME_FOUNDATION_API extern uint fps;
	FLAME_FOUNDATION_API extern float delta_time; // second
	FLAME_FOUNDATION_API extern float total_time; // second
	FLAME_FOUNDATION_API extern bool app_exiting;

	FLAME_FOUNDATION_API int run(const std::function<bool()>& callback);

	FLAME_FOUNDATION_API void* add_event(const std::function<bool()>& callback, float time = 0.f);
	FLAME_FOUNDATION_API void reset_event(void* ev);
	FLAME_FOUNDATION_API void remove_event(void* ev);
	FLAME_FOUNDATION_API void clear_events();
}
