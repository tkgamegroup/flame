#pragma once

#ifdef FLAME_WINDOWS
#ifdef FLAME_FOUNDATION_MODULE
#define FLAME_FOUNDATION_EXPORTS __declspec(dllexport)
#else
#define FLAME_FOUNDATION_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_FOUNDATION_EXPORTS
#endif

#include <flame/math.h>

#include <chrono>
#include <thread>
#include <mutex>

#define FLAME_R(name, ...) name
#define FLAME_RV(type, name, ...) type name;
#define FLAME_RF(name, ...) name

namespace flame
{
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

	struct Guid
	{
		uint d1;
		ushort d2;
		ushort d3;
		uchar d4[8];
	};

	template <class CH>
	struct String
	{
		uint s;
		CH* v;

		String() :
			s(0),
			v(nullptr)
		{
		}

		String(const String& rhs)
		{
			s = rhs.s;
			v = (CH*)f_malloc(sizeof(CH) * (s + 1));
			memcpy(v, rhs.v, sizeof(CH) * s);
			v[s] = 0;
		}

		String(String&& rhs)
		{
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		String(const CH* str, uint _s)
		{
			s = _s;
			v = (CH*)f_malloc(sizeof(CH) * (s + 1));
			memcpy(v, str, sizeof(CH) * s);
			v[s] = 0;
		}

		String(const CH* str) :
			String(str, std::char_traits<CH>::length(str))
		{
		}

		String(const std::basic_string<CH>& str) :
			String(str.data(), str.size())
		{
		}

		~String()
		{
			f_free(v);
		}

		void resize(uint _s)
		{
			if (s != _s || (s == 0 && v == nullptr))
			{
				s = _s;
				v = (CH*)f_realloc(v, sizeof(CH) * (s + 1));
				v[s] = 0;
			}
		}

		void assign(const CH* _v, uint _s)
		{
			resize(_s);
			memcpy(v, _v, sizeof(CH) * s);
		}

		void operator=(const String& rhs)
		{
			assign(rhs.v, rhs.s);
		}

		void operator=(String&& rhs)
		{
			f_free(v);
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		void operator=(const CH* str)
		{
			assign(str, std::char_traits<CH>::length(str));
		}

		void operator=(const std::basic_string<CH>& str)
		{
			assign(str.c_str(), str.size());
		}

		bool operator==(const std::basic_string<CH>& str)
		{
			return s == str.size() && std::char_traits<CH>::compare(v, str.c_str(), s) == 0;
		}

		std::basic_string<CH> str() const
		{
			return std::basic_string<CH>(v, s);
		}

		bool compare(const CH* str, uint len)
		{
			if (s != len)
				return false;
			return std::char_traits<CH>::compare(v, str, len) == 0;
		}
	};

	using StringA = String<char>;
	using StringW = String<wchar_t>;

	struct StringAH : StringA
	{
		uint h;

		StringAH() :
			StringA()
		{
			h = 0;
		}

		StringAH(const StringA& rhs) :
			StringA(rhs)
		{
			h = FLAME_HASH(v);
		}

		StringAH(StringA&& rhs) :
			StringA(rhs)
		{
			h = FLAME_HASH(v);
		}

		StringAH(const char* str, uint _s) :
			StringA(str, _s)
		{
			h = FLAME_HASH(v);
		}

		StringAH(const char* str) :
			StringA(str)
		{
			h = FLAME_HASH(v);
		}

		StringAH(const std::basic_string<char>& str) :
			StringA(str)
		{
			h = FLAME_HASH(v);
		}

		void operator=(const StringA& rhs)
		{
			StringA::operator=(rhs);
			h = FLAME_HASH(v);
		}

		void operator=(StringA&& rhs)
		{
			StringA::operator=(rhs);
			h = FLAME_HASH(v);
		}

		void operator=(const char* str)
		{
			StringA::operator=(str);
			h = FLAME_HASH(v);
		}

		void operator=(const std::basic_string<char>& str)
		{
			StringA::operator=(str);
			h = FLAME_HASH(v);
		}
	};

	template <class T>
	struct Array
	{
		struct Iterator
		{
			T* ptr;

			Iterator(T* ptr) : 
				ptr(ptr) 
			{
			}

			Iterator operator++() 
			{ 
				++ptr; 
				return *this; 
			}

			bool operator!=(const Iterator& other) const
			{ 
				return ptr != other.ptr; 
			}

			const T& operator*() const 
			{ 
				return *ptr; 
			}

		};

		uint s;
		T* v;

		Array() :
			s(0),
			v(nullptr)
		{
		}

		Array(const Array& rhs)
		{
			s = rhs.s;
			v = (T*)f_malloc(sizeof(T) * s);
			for (auto i = 0; i < s; i++)
			{
				new (&v[i]) T;
				v[i] = rhs.v[i];
			}
		}

		Array(Array&& rhs)
		{
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		~Array()
		{
			for (auto i = 0; i < s; i++)
				v[i].~T();
			f_free(v);
		}

		void resize(uint _s)
		{
			if (s != _s)
			{
				for (auto i = _s; i < s; i++)
					v[i].~T();
				if (_s != 0)
				{
					v = (T*)f_realloc(v, sizeof(T) * _s);
					for (auto i = s; i < _s; i++)
						new (&v[i])T;
				}
				else
				{
					f_free(v);
					v = nullptr;
				}
				s = _s;
			}
		}

		const T& at(uint idx) const
		{
			return v[idx];
		}

		T& at(uint idx)
		{
			return v[idx];
		}

		const T& operator[](uint idx) const
		{
			return v[idx];
		}

		T& operator[](uint idx)
		{
			return v[idx];
		}

		void operator=(const Array& rhs)
		{
			resize(rhs.s);
			for (auto i = 0; i < s; i++)
				v[i] = rhs.v[i];
		}

		void operator=(Array&& rhs)
		{
			for (auto i = 0; i < s; i++)
				v[i].~T();
			f_free(v);
			s = rhs.s;
			v = rhs.v;
			rhs.s = 0;
			rhs.v = nullptr;
		}

		Iterator begin() const
		{
			return Iterator(v);
		}

		Iterator end() const
		{
			return Iterator(v + s);
		}

		void insert(uint pos, const T& _v)
		{
			assert(pos <= s);
			resize(s + 1);
			for (auto i = s - 1; i > pos; i--)
				v[i] = v[i - 1];
			v[pos] = _v;
		}

		void push_back(const T& _v)
		{
			insert(s, _v);
		}

		void remove(uint offset, uint count = 1)
		{
			for (auto i = offset; i < s - count; i++)
				v[i] = v[i + count];
			resize(s - count);
		}
	};

	struct Object
	{
		const char* name;
		const uint name_hash;
		uint id;
		uint debug_level;

		Object(const char* name) :
			name(name),
			name_hash(FLAME_HASH(name)),
			id(0),
			debug_level(0)
		{
		}
	};

	struct Capture
	{
		uint size;
		void* _data;
		void* _thiz;
		void* _current;

		Capture() :
			size(0),
			_data(nullptr),
			_thiz(nullptr),
			_current(nullptr)
		{
		}

		template <class T>
		Capture& set_data(T* p)
		{
			assert(!_data);
			size = sizeof(T);
			_data = f_malloc(size);
			memcpy(_data, p, size);
			return *this;
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

	template <class F>
	struct Closure
	{
		F* f;
		Capture c;

		Closure(F* f, const Capture& c) :
			f(f),
			c(c)
		{
		}

		~Closure()
		{
			f_free(c._data);
		}

		template <class FF = F, class ...Args>
		auto call(Args... args)
		{
			return ((FF*)f)(c, args...);
		}
	};

	struct ListenerHubImpl
	{
		virtual void release() = 0;

		virtual uint count() const = 0;
		virtual Closure<bool(Capture&)>& item(uint idx) const = 0;
		virtual void* add(bool(*pf)(Capture& c), const Capture& capture, int pos) = 0;
		virtual void remove(void* l) = 0;

		FLAME_FOUNDATION_EXPORTS static ListenerHubImpl* create();
	};

	template <class F>
	struct ListenerHub
	{
		ListenerHubImpl* impl;

		void* add(F* pf, const Capture& capture, int pos = -1)
		{
			return impl->add((bool(*)(Capture& c))pf, capture, pos);
		}

		void remove(void* l)
		{
			impl->remove(l);
		}

		template <class ...Args>
		void call(Args... args)
		{
			auto count = impl->count();
			for (auto i = 0; i < count; i++)
			{
				if (!impl->item(i).call<F>(args...))
					break;
			}
		}

		template <class ...Args>
		void call_no_check(Args... args)
		{
			auto count = impl->count();
			for (auto i = 0; i < count; i++)
				impl->item(i).call<F>(args...);
		}

		template <class ...Args>
		void call_with_current(void* current, Args... args)
		{
			auto count = impl->count();
			for (auto i = 0; i < count; i++)
			{
				auto& closure = impl->item(i);
				closure.c._current = current;
				auto pass = closure.call<F>(args...);
				closure.c._current = nullptr;
				if (!pass)
					break;
			}
		}

		template <class ...Args>
		void call_no_check_with_current(void* current, Args... args)
		{
			auto count = impl->count();
			for (auto i = 0; i < count; i++)
			{
				auto& closure = impl->item(i);
				closure.c._current = current;
				closure.call<F>(args...);
				closure.c._current = nullptr;
			}
		}
	};

	enum KeyState
	{
		KeyStateNull,
		KeyStateUp = 1 << 0,
		KeyStateDown = 1 << 1,
		KeyStateJust = 1 << 2,
		KeyStateDouble = 1 << 2,
	};

	typedef uint KeyStateFlags;

	enum Key
	{
		KeyNull = -1,

		Key_Backspace,
		Key_Tab,
		Key_Enter,
		Key_Shift,
		Key_Ctrl,
		Key_Alt,
		Key_Pause,
		Key_CapsLock,
		Key_Esc,
		Key_Space,
		Key_PgUp, Key_PgDn,
		Key_End,
		Key_Home,
		Key_Left, Key_Up, Key_Right, Key_Down,
		Key_PrtSc,
		Key_Ins,
		Key_Del,
		Key_0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
		Key_A, Key_B, Key_C, Key_D, Key_E, Key_F, Key_G, Key_H, Key_I, Key_J, Key_K, Key_L, Key_M, Key_N, Key_O, Key_P, Key_Q, Key_R, Key_S, Key_T, Key_U, Key_V, Key_W, Key_X, Key_Y, Key_Z,
		Key_Numpad0, Key_Numpad1, Key_Numpad2, Key_Numpad3, Key_Numpad4, Key_Numpad5, Key_Numpad6, Key_Numpad7, Key_Numpad8, Key_Numpad9,
		Key_Add, Key_Subtract, Key_Multiply, Key_Divide,
		Key_Separator,
		Key_Decimal,
		Key_F1, Key_F2, Key_F3, Key_F4, Key_F5, Key_F6, Key_F7, Key_F8, Key_F9, Key_F10, Key_F11, Key_F12,
		Key_NumLock,
		Key_ScrollLock,

		Key_Count
	};

	enum MouseKey
	{
		Mouse_Null = -1,
		Mouse_Left,
		Mouse_Right,
		Mouse_Middle,

		MouseKey_Count
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

	inline bool is_key_down(KeyStateFlags action) // value is Key
	{
		return action == KeyStateDown;
	}

	inline bool is_key_up(KeyStateFlags action) // value is Key
	{
		return action == KeyStateUp;
	}

	inline bool is_key_char(KeyStateFlags action) // value is ch
	{
		return action == KeyStateNull;
	}

	inline bool is_mouse_enter(KeyStateFlags action, MouseKey key)
	{
		return action == KeyStateDown && key == Mouse_Null;
	}

	inline bool is_mouse_leave(KeyStateFlags action, MouseKey key)
	{
		return action == KeyStateUp && key == Mouse_Null;
	}

	inline bool is_mouse_down(KeyStateFlags action, MouseKey key, bool just = false) // value is pos
	{
		return action == (KeyStateDown | (just ? KeyStateJust : 0)) && key != Mouse_Null;
	}

	inline bool is_mouse_up(KeyStateFlags action, MouseKey key, bool just = false) // value is pos
	{
		return action == (KeyStateUp | (just ? KeyStateJust : 0)) && key != Mouse_Null;
	}

	inline bool is_mouse_move(KeyStateFlags action, MouseKey key) // value is disp
	{
		return action == KeyStateNull && key == Mouse_Null;
	}

	inline bool is_mouse_scroll(KeyStateFlags action, MouseKey key) // value.x() is scroll value
	{
		return action == KeyStateNull && key == Mouse_Middle;
	}

	inline bool is_mouse_clicked(KeyStateFlags action, MouseKey key)
	{
		return (action & KeyStateDown) && (action & KeyStateUp) && key == Mouse_Null;
	}

	inline uint64 get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	FLAME_FOUNDATION_EXPORTS Guid generate_guid();
	FLAME_FOUNDATION_EXPORTS void set_engine_path(const wchar_t* p);
	FLAME_FOUNDATION_EXPORTS const wchar_t* get_engine_path();
	FLAME_FOUNDATION_EXPORTS void set_file_callback(void(*callback)(Capture& c, const wchar_t* filename), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void report_used_file(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS StringW get_curr_path();
	FLAME_FOUNDATION_EXPORTS StringW get_app_path(bool has_name = false);
	FLAME_FOUNDATION_EXPORTS void set_curr_path(const wchar_t* p);
	FLAME_FOUNDATION_EXPORTS void* load_library(const wchar_t* library_name);
	FLAME_FOUNDATION_EXPORTS void* get_library_func(void* library, const char* name);
	FLAME_FOUNDATION_EXPORTS void free_library(void* library);
	FLAME_FOUNDATION_EXPORTS void* get_hinst();
	FLAME_FOUNDATION_EXPORTS Vec2u get_screen_size();
	FLAME_FOUNDATION_EXPORTS void read_process_memory(void* process, void* address, uint size, void* dst);
	FLAME_FOUNDATION_EXPORTS void sleep(int time); // a time less than 0 means forever
	FLAME_FOUNDATION_EXPORTS void* create_event(bool signaled, bool manual = false);
	FLAME_FOUNDATION_EXPORTS void set_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void reset_event(void* ev);
	FLAME_FOUNDATION_EXPORTS bool wait_event(void* ev, int timeout);
	FLAME_FOUNDATION_EXPORTS void destroy_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void debug_break();
	FLAME_FOUNDATION_EXPORTS void do_simple_dispatch_loop(void(callback)(Capture& c) = nullptr, const Capture& capture = Capture());
	FLAME_FOUNDATION_EXPORTS bool is_file_occupied(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void exec(const wchar_t* filename, wchar_t* parameters, bool wait, bool show = false);
	FLAME_FOUNDATION_EXPORTS StringA exec_and_get_output(const wchar_t* filename, wchar_t* parameters);
	FLAME_FOUNDATION_EXPORTS void exec_and_redirect_to_std_output(const wchar_t* filename, wchar_t* parameters);
	FLAME_FOUNDATION_EXPORTS StringW get_library_name(void* library);
	FLAME_FOUNDATION_EXPORTS void* get_library_from_address(void* addr);
	FLAME_FOUNDATION_EXPORTS Array<StringW> get_library_dependencies(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS StringW get_clipboard();
	FLAME_FOUNDATION_EXPORTS void set_clipboard(const wchar_t* s);
	FLAME_FOUNDATION_EXPORTS void open_explorer_and_select(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void move_to_trashbin(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void get_thumbnail(uint width, const wchar_t* filename, uint* out_width, uint* out_height, char** out_data);
	FLAME_FOUNDATION_EXPORTS Key vk_code_to_key(int vkCode);
	FLAME_FOUNDATION_EXPORTS bool is_modifier_pressing(Key k /* accept: Key_Shift, Key_Ctrl and Key_Alt */, int left_or_right /* 0 or 1 */);
	FLAME_FOUNDATION_EXPORTS void* add_global_key_listener(Key key, bool modifier_shift, bool modifier_ctrl, bool modifier_alt, void (*callback)(Capture& c, KeyStateFlags action), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void remove_global_key_listener(void* handle/* return by add_global_key_listener */);
	FLAME_FOUNDATION_EXPORTS void send_global_key_event(KeyState action, Key key);
	FLAME_FOUNDATION_EXPORTS void send_global_mouse_event(KeyState action, MouseKey key);

	struct StackFrameInfo
	{
		StringA file;
		uint line;
		StringA function;
	};

	FLAME_FOUNDATION_EXPORTS Array<void*> get_stack_frames();
	FLAME_FOUNDATION_EXPORTS Array<StackFrameInfo> get_stack_frame_infos(uint frames_count, void** frames);

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_FOUNDATION_EXPORTS void* /* event */ add_file_watcher(const wchar_t* path, void (*callback)(Capture& c, FileChangeType type, const wchar_t* filename), const Capture& capture, bool all_changes = true, bool sync = true);
	// set_event to returned ev to end the file watching

	FLAME_FOUNDATION_EXPORTS void add_work(void (*function)(Capture& c), const Capture& capture);
	FLAME_FOUNDATION_EXPORTS void clear_all_works();
	FLAME_FOUNDATION_EXPORTS void wait_all_works();

	enum WindowStyle
	{
		WindowFrame = 1 << 0,
		WindowResizable = 1 << 1,
		WindowFullscreen = 1 << 2,
		WindowMaximized = 1 << 3,
		WindowTopmost = 1 << 4
	};

	typedef uint WindowStyleFlags;

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

	struct Window;

	struct Window : Object
	{
		Window() :
			Object("Window")
		{
		}

		virtual void release() = 0;

		virtual void* get_native() = 0;

		virtual Vec2i get_pos() const = 0;
		virtual void set_pos(const Vec2i& pos) = 0;
		virtual Vec2u get_size() const = 0;
		virtual void set_size(const Vec2u& size) = 0;

		virtual const char* get_title() const = 0;
		virtual void set_title(const char* title) = 0;

		virtual int get_style() const = 0;

		virtual CursorType get_cursor() = 0;
		virtual void set_cursor(CursorType type) = 0;

		virtual void close() = 0;

		virtual void* add_key_listener(void (*callback)(Capture& c, KeyStateFlags action, int value), const Capture& capture) = 0;
		virtual void remove_key_listener(void* lis) = 0;
		virtual void* add_mouse_listener(void (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_listener(void* lis) = 0;
		virtual void* add_resize_listener(void (*callback)(Capture& c, const Vec2u& size), const Capture& capture) = 0;
		virtual void remove_resize_listener(void* lis) = 0;
		virtual void* add_destroy_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_destroy_listener(void* lis) = 0;

		FLAME_FOUNDATION_EXPORTS static Window* create(const char* title, const Vec2u& size, WindowStyleFlags style, Window* parent = nullptr);
	};

	struct Looper
	{
		uint frame;
		float delta_time; // second
		float total_time; // second

		FLAME_FOUNDATION_EXPORTS int loop(void (*frame_callback)(Capture& c), const Capture& capture);

		FLAME_FOUNDATION_EXPORTS void* add_event(void (*callback)(Capture& c /* set c._current to invalid to keep event */ ), const Capture& capture, CountDown interval = CountDown(), uint id = 0);
		FLAME_FOUNDATION_EXPORTS void reset_event(void* ev);
		FLAME_FOUNDATION_EXPORTS void remove_event(void* ev);
		FLAME_FOUNDATION_EXPORTS void remove_events(int id = 0); /* id=-1 means all */
		FLAME_FOUNDATION_EXPORTS void process_events();
	};

	FLAME_FOUNDATION_EXPORTS Looper& looper();

	struct Schedule
	{
		FLAME_FOUNDATION_EXPORTS void add_event(float delay, float duration, void(*callback)(Capture& c, float time, float duration), const Capture& capture);
		FLAME_FOUNDATION_EXPORTS void begin_group();
		FLAME_FOUNDATION_EXPORTS void end_group();
		FLAME_FOUNDATION_EXPORTS void start();
		FLAME_FOUNDATION_EXPORTS void stop();

		FLAME_FOUNDATION_EXPORTS static Schedule* create(bool once = true);
		FLAME_FOUNDATION_EXPORTS static void destroy(Schedule* s);
	};
}
