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
	typedef FlameFoundationTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../serialize.h"

#include <chrono>
#include <thread>
#include <mutex>

namespace flame
{
	FLAME_FOUNDATION_TYPE(NativeWindow);
	FLAME_FOUNDATION_TYPE(Looper);
	FLAME_FOUNDATION_TYPE(Schedule);

	FLAME_FOUNDATION_TYPE(Bitmap);

	FLAME_FOUNDATION_TYPE(TypeInfo);
	FLAME_FOUNDATION_TYPE(ReflectMeta);
	FLAME_FOUNDATION_TYPE(VariableInfo);
	FLAME_FOUNDATION_TYPE(EnumItemInfo);
	FLAME_FOUNDATION_TYPE(EnumInfo);
	FLAME_FOUNDATION_TYPE(FunctionInfo);
	FLAME_FOUNDATION_TYPE(UdtInfo);
	FLAME_FOUNDATION_TYPE(TypeInfoDataBase);

	FLAME_FOUNDATION_EXPORTS void* f_malloc(uint size);
	FLAME_FOUNDATION_EXPORTS void* f_realloc(void* p, uint size);
	FLAME_FOUNDATION_EXPORTS void f_free(void* p);

	template <class T, class ...Args>
	T* f_new(Args... args)
	{
		auto ret = (T*)f_malloc(sizeof(T));
		new (ret) T(args...);
		return ret;
	}

	template <class T>
	void f_delete(T* p)
	{
		p->~T();
		f_free(p);
	}

	struct Delector
	{
		template <class T>
		void operator()(T* p)
		{
			f_delete(p);
		}
	};

	template <class T>
	struct UniPtr
	{
		T* p = nullptr;

		UniPtr()
		{
		}

		UniPtr(T* p) : 
			p(p)
		{
		}

		~UniPtr()
		{
			if (p)
				p->release();
		}

		UniPtr(UniPtr&& oth)
		{
			oth.swap(*this);
		}

		UniPtr& operator=(UniPtr&& oth)
		{
			oth.swap(*this);
			return *this;
		}

		T** operator&()
		{
			return &p;
		}

		UniPtr(UniPtr const&) = delete;
		UniPtr& operator=(UniPtr const&) = delete; 

		T* operator->() const { return p; }
		T& operator*()  const { return *p; }
		T* get() const { return p; }
		explicit operator bool() const { return p; }

		T* release()
		{
			auto temp = p;
			p = nullptr;
			return temp;
		}

		void swap(UniPtr& oth) noexcept
		{
			std::swap(p, oth.p);
		}

		void reset(T* _p = nullptr)
		{
			if (p)
				p->release();
			p = _p;
		}
	};

	FLAME_FOUNDATION_EXPORTS void raise_assert(const char* expression, const char* file, uint line);

#ifdef NDEBUG
#define fassert(expression)
#else
#define fassert(expression) ((!!(expression)) || (raise_assert(#expression, __FILE__, __LINE__), 0))
#endif

	struct Guid
	{
		uint d1;
		ushort d2;
		ushort d3;
		uchar d4[8];
	};

	struct Capture
	{
		uint size = 0;
		void* _data = nullptr;
		void* _thiz = nullptr;
		void* _current = nullptr;

		Capture() {}

		Capture& set_data(uint s, void* p)
		{
			assert(!_data);
			size = s;
			_data = f_malloc(size);
			memcpy(_data, p, size);
			return *this;
		}

		template <class T>
		Capture& set_data(T* p)
		{
			return set_data(sizeof(T), p);
		}

		Capture& set_thiz(void* thiz)
		{
			assert(!_thiz);
			_thiz = thiz;
			return *this;
		}

		template <class T>
		Capture& absorb(T* p, const Capture& original, bool kill_original = false)
		{
			assert(!_data);
			size = sizeof(T) + original.size;
			_data = f_malloc(size);
			memcpy(_data, p, sizeof(T));
			memcpy((char*)_data + sizeof(T), original._data, original.size);
			_thiz = original._thiz;
			if (kill_original)
				f_free(original._data);
			return *this;
		}

		template <class T>
		Capture release()
		{
			auto ret = Capture();
			ret.size = size - sizeof(T);
			ret._data = (char*)_data + sizeof(T);
			ret._thiz = _thiz;
			ret._current = _current;
			return ret;
		}

		template <class T>
		T& data()
		{
			return *(T*)_data;
		}

		template <class T>
		T* thiz()
		{
			return (T*)_thiz;
		}

		template <class T>
		T* current()
		{
			return (T*)_current;
		}
	};

	template <class Function>
	struct Closure
	{
		Function* f;
		Capture c;

		Closure(Function* f, const Capture& c) :
			f(f),
			c(c)
		{
		}

		~Closure()
		{
			f_free(c._data);
		}

		template <class FF = Function, class ...Args>
		auto call(Args... args)
		{
			return ((FF*)f)(c, args...);
		}
	};

	template <class Function>
	struct ListenerManagement
	{
		std::vector<std::unique_ptr<Closure<Function>>> list;
		bool staging = false;
		std::vector<std::unique_ptr<Closure<Function>>> staging_adds;
		std::vector<void*> staging_removes;

		void* add(Function *f, const Capture& capture)
		{
			auto c = new Closure(f, capture);
			if (staging)
				staging_adds.emplace_back(c);
			else
				list.emplace_back(c);
			return c;
		}

		void remove(void* c)
		{
			if (staging)
				staging_removes.push_back(c);
			else
			{
				std::erase_if(list, [&](const auto& i) {
					return i.get() == (decltype(i.get()))c;
				});
			}
		}

		void begin_staging()
		{
			fassert(!staging);

			staging = true;
		}

		void end_staging()
		{
			fassert(staging);

			staging = false;
			for (auto& c : staging_adds)
				list.push_back(std::move(c));
			staging_adds.clear();
			for (auto c : staging_removes)
			{
				std::erase_if(list, [&](const auto& i) {
					return i.get() == (decltype(i.get()))c;
				});
			}
			staging_removes.clear();
		}
	};

	inline bool get_engine_path(std::filesystem::path& path, const std::filesystem::path& subdir)
	{
		if (!std::filesystem::exists(path))
		{
			auto engine_path = getenv("FLAME_PATH");
			if (engine_path)
				path = std::filesystem::path(engine_path) / subdir / path;
			if (!std::filesystem::exists(path))
				return false;
		}
		return true;
	}

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

	enum GeneralFormula
	{
		GeneralFormula_None,
		GeneralFormula_v_mul_a_add_b
	};

	FLAME_FOUNDATION_EXPORTS float apply_general_formula(float v, const vec4& f);

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

	inline uint64 get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>
			(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	FLAME_FOUNDATION_EXPORTS Guid generate_guid();
	FLAME_FOUNDATION_EXPORTS void get_current_path(wchar_t* path);
	FLAME_FOUNDATION_EXPORTS void set_current_path(const wchar_t* path);
	FLAME_FOUNDATION_EXPORTS void get_app_path(wchar_t* dst, bool has_name = false);
	FLAME_FOUNDATION_EXPORTS void get_logical_drives(uint *count, wchar_t** names);
	FLAME_FOUNDATION_EXPORTS void* get_hinst();
	FLAME_FOUNDATION_EXPORTS uvec2 get_screen_size();
	FLAME_FOUNDATION_EXPORTS void* create_event(bool signaled, bool manual = false);
	FLAME_FOUNDATION_EXPORTS void set_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void reset_event(void* ev);
	FLAME_FOUNDATION_EXPORTS bool wait_event(void* ev, int timeout);
	FLAME_FOUNDATION_EXPORTS void destroy_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void get_module_dependencies(const wchar_t* filename, void (*callback)(Capture& c, const wchar_t* filename), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void get_clipboard(void* str, wchar_t* (*str_allocator)(void* str, uint size));
	FLAME_FOUNDATION_EXPORTS void set_clipboard(const wchar_t* s);
	FLAME_FOUNDATION_EXPORTS void get_thumbnail(uint width, const wchar_t* filename, uint* out_width, uint* out_height, uchar** out_data);
	FLAME_FOUNDATION_EXPORTS bool is_keyboard_key_pressing(KeyboardKey key);
	FLAME_FOUNDATION_EXPORTS void* add_global_key_listener(KeyboardKey key, void (*callback)(Capture& c), const Capture& capture, bool down = true, bool ctrl = false, bool shift = false, bool alt = false);
	FLAME_FOUNDATION_EXPORTS void remove_global_key_listener(void* ret);
	FLAME_FOUNDATION_EXPORTS void send_global_keyboard_event(KeyboardKey key, bool down = true);
	FLAME_FOUNDATION_EXPORTS void send_global_mouse_event(MouseKey key, bool down = true);
	FLAME_FOUNDATION_EXPORTS void set_mouse_pos(const ivec2& pos);
	FLAME_FOUNDATION_EXPORTS void shell_exec(const wchar_t* filename, wchar_t* parameters, bool wait, bool show = false);
	// if str is null then the output will be redirect to std output
	FLAME_FOUNDATION_EXPORTS void exec(const wchar_t* filename, wchar_t* parameters, char* output = nullptr);
	FLAME_FOUNDATION_EXPORTS void debug_break();
	FLAME_FOUNDATION_EXPORTS void* add_assert_callback(void (*callback)(Capture& c), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void remove_assert_callback(void* ret);

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

	struct StackFrameInfo
	{
		char file[260];
		uint line;
		char function[260];
	};

	// max depth: 64
	FLAME_FOUNDATION_EXPORTS void get_call_frames(void** (*array_allocator)(Capture& c, uint size), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void get_call_frames_infos(uint frames_count, void** frames, StackFrameInfo* dst);

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_FOUNDATION_EXPORTS void* /* event */ add_file_watcher(const wchar_t* path, void (*callback)(Capture& c, FileChangeType type, const wchar_t* filename), const Capture& capture, bool all_changes = true, bool sync = true);
	// set_event to the returned ev to end the file watching

	FLAME_FOUNDATION_EXPORTS void add_work(void (*function)(Capture& c), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void clear_all_works();
	FLAME_FOUNDATION_EXPORTS void wait_all_works();

	enum NativeWindowStyleFlags
	{
		NativeWindowFrame = 1 << 0,
		NativeWindowResizable = 1 << 1,
		NativeWindowFullscreen = 1 << 2,
		NativeWindowMaximized = 1 << 3,
		NativeWindowTopmost = 1 << 4
	};

	inline NativeWindowStyleFlags operator| (NativeWindowStyleFlags a, NativeWindowStyleFlags b) { return (NativeWindowStyleFlags)((int)a | (int)b); }

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

	struct NativeWindow
	{
		virtual void release() = 0;

		virtual void* get_native() = 0;

		virtual ivec2 get_pos() const = 0;
		virtual void set_pos(const ivec2& pos) = 0;
		virtual uvec2 get_size() const = 0;
		virtual void set_size(const uvec2& size) = 0;

		virtual ivec2 global_to_local(const ivec2& p) = 0;

		virtual const wchar_t* get_title() const = 0;
		virtual void set_title(const wchar_t* title) = 0;

		virtual int get_style() const = 0;

		virtual CursorType get_cursor() = 0;
		virtual void set_cursor(CursorType type) = 0;

		virtual void* add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) = 0;
		virtual void remove_key_down_listener(void* lis) = 0;
		virtual void* add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) = 0;
		virtual void remove_key_up_listener(void* lis) = 0;
		virtual void* add_char_listener(void (*callback)(Capture& c, wchar_t ch), const Capture& capture) = 0;
		virtual void remove_char_listener(void* lis) = 0;
		virtual void* add_mouse_left_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_left_down_listener(void* lis) = 0;
		virtual void* add_mouse_left_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_left_up_listener(void* lis) = 0;
		virtual void* add_mouse_right_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_right_down_listener(void* lis) = 0;
		virtual void* add_mouse_right_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_right_up_listener(void* lis) = 0;
		virtual void* add_mouse_middle_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_middle_down_listener(void* lis) = 0;
		virtual void* add_mouse_middle_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_middle_up_listener(void* lis) = 0;
		virtual void* add_mouse_move_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_move_listener(void* lis) = 0;
		virtual void* add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture) = 0;
		virtual void remove_mouse_scroll_listener(void* lis) = 0;
		virtual void* add_resize_listener(void (*callback)(Capture& c, const uvec2& size), const Capture& capture) = 0;
		virtual void remove_resize_listener(void* lis) = 0;
		virtual void* add_destroy_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_destroy_listener(void* lis) = 0;

		FLAME_FOUNDATION_EXPORTS static NativeWindow* create(const wchar_t* title, const uvec2& size, NativeWindowStyleFlags style, NativeWindow* parent = nullptr);
	};

	struct Looper
	{
		virtual uint get_frame() const = 0;
		virtual float get_delta_time() const = 0; // second
		virtual float get_total_time() const = 0; // second
		virtual uint get_fps() const = 0;

		virtual int loop(void (*frame_callback)(Capture& c, float delta_time) = nullptr, const Capture& capture = {}) = 0;

		/* set c._current to null to keep event */
		virtual void* add_event(void (*callback)(Capture& c), const Capture& capture, CountDown interval = CountDown(), uint id = 0) = 0;
		virtual void reset_event(void* ev) = 0;
		virtual void remove_event(void* ev) = 0;
		virtual void remove_events(int id = 0) = 0; /* id=-1 means all */
	};

	FLAME_FOUNDATION_EXPORTS Looper& looper();
}
