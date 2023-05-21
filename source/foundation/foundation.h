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

	enum WindowStyleFlags
	{
		WindowFrame = 1 << 0,
		WindowResizable = 1 << 1,
		WindowFullscreen = 1 << 2,
		WindowMaximized = 1 << 3,
		WindowTopmost = 1 << 4
	};

	inline WindowStyleFlags operator| (WindowStyleFlags a, WindowStyleFlags b) { return (WindowStyleFlags)((int)a | (int)b); }

	enum CursorType
	{
		CursorNone = -1,
		CursorAppStarting, // arrow and small hourglass
		CursorArrow,
		CursorCross, // unknown
		CursorHand,
		CursorHelp,
		CursorIBeam,
		CursorNo,
		CursorSizeAll,
		CursorSizeNESW,
		CursorSizeNS,
		CursorSizeNWSE,
		CursorSizeWE,
		CursorUpArrwo,
		CursorWait,

		Cursor_Count
	};

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
		Keyboard_1, Keyboard_2, Keyboard_3, Keyboard_4, Keyboard_5, Keyboard_6, Keyboard_7, Keyboard_8, Keyboard_9, Keyboard_0,
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

	struct GUID
	{
		char d[16];

		inline std::string to_string()
		{
			std::string ret;
			ret += str_hex(*(uint64*)(d));
			ret += str_hex(*(uint64*)(d + sizeof(uint64)));
			return ret;
		}

		inline void from_string(const std::string& str)
		{
			*(uint64*)d = s2u_hex<uint64>(str.substr(0, sizeof(uint64) * 2));
			*(uint64*)(d + sizeof(uint64)) = s2u_hex<uint64>(str.substr(sizeof(uint64) * 2, sizeof(uint64) * 2));
		}
	};

	inline bool operator==(const GUID& a, const GUID& b)
	{
		return *(uint64*)a.d == *(uint64*)b.d && *(uint64*)(a.d + sizeof(uint64)) == *(uint64*)(b.d + sizeof(uint64));
	}

	inline uint64 current_time()
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		);
		return ms.count();
	}

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
			if (path.empty())
				return L"";
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
				if (str1.starts_with(str2))
				{
					if (str1.size() == str2.size())
						return r.first;
					return r.first / std::filesystem::path(str1.substr(str2.size() + 1));
				}
			}
			return path;
		}

		inline static bool exists(const std::filesystem::path& path)
		{
			if (auto pos = path.native().find('#'); pos != std::wstring::npos)
				return std::filesystem::exists(path.native().substr(0, pos));
			return std::filesystem::exists(path);
		}

		inline static std::filesystem::path combine(const std::filesystem::path& base, const std::filesystem::path& path)
		{
			auto ret = base / path;
			if (exists(Path::get(ret)))
				return ret;
			return path;
		}

		inline static std::filesystem::path rebase(const std::filesystem::path& base, const std::filesystem::path& path)
		{
			auto& str1 = base.native();
			auto& str2 = path.native();
			if (str1 == str2)
				return L"";
			if (str1.size() >= str2.size())
				return path;
			for (auto i = 0; i < str1.size(); i++)
			{
				auto ch1 = str1[i];
				auto ch2 = str2[i];
				if (ch1 == ch2)
					continue;
				if (ch1 == '\\' || ch1 == '/')
				{
					if (ch2 == '\\' || ch2 == '/')
						continue;
				}
				return path;
			}
			return str2.substr(str1.size() + 1);
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
		std::unique_ptr<std::vector<std::pair<std::function<F>, uint>>> list;

		operator bool()
		{
			return list && !list->empty();
		}

		void add(const std::function<F>& callback, uint h = 0, int pos = -1)
		{
			if (!list)
				list.reset(new std::vector<std::pair<std::function<F>, uint>>);
			if (h != 0)
			{
				if (find(h) != -1)
					return;
			}
			if (pos == -1)
				pos = list->size();
			list->emplace(list->begin() + pos, std::make_pair(callback, h));
		}

		void remove(uint h)
		{
			if (!list)
				return;
			std::erase_if(*list, [&](const auto& i) {
				return i.second == h;
			});
		}

		int find(uint h)
		{
			if (!list)
				return -1;
			for (auto i = 0; i < list->size(); i++)
			{
				if (list->at(i).second == h)
					return i;
			}
			return -1;
		}

		template<typename... Args>
		void call(Args ...args) const
		{
			if (!list)
				return;
			for (auto& l : *list)
				l.first(args...);
		}
	};

	struct AssetManagemant
	{
		struct Asset
		{
			uint type;
			void* obj;

			std::filesystem::file_time_type lwt;

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

		FLAME_FOUNDATION_API static Asset& get(const std::filesystem::path& path);
		FLAME_FOUNDATION_API static void release(const std::filesystem::path& path);
	};

	struct Expression
	{
		std::string expression_string;

		virtual ~Expression() {}

		virtual void set_const_value(const std::string& name, float value) = 0;
		virtual void set_variable(const std::string& name, float* variable) = 0;
		virtual void set_const_string(const std::string& name, const std::string& value) = 0;
		virtual void compile() = 0;
		virtual std::string get_value() = 0;

		FLAME_FOUNDATION_API static Expression* create(const std::string& expression_string);
	};

	FLAME_FOUNDATION_API extern uint frames;
	FLAME_FOUNDATION_API extern uint fps;
	FLAME_FOUNDATION_API extern float delta_time; // second
	FLAME_FOUNDATION_API extern float total_time; // second
	FLAME_FOUNDATION_API extern bool app_exiting;

	FLAME_FOUNDATION_API int run(const std::function<bool()>& callback);

	FLAME_FOUNDATION_API void* add_event(const std::function<bool /* return false to end the event*/()>& callback, float time = 0.f, uint frames = 0);
	FLAME_FOUNDATION_API void reset_event(void* ev);
	FLAME_FOUNDATION_API void remove_event(void* ev);
	FLAME_FOUNDATION_API void clear_events();
}
