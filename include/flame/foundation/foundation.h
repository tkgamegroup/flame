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

#ifdef FLAME_WINDOWS
#define LOGI(...) {printf(__VA_ARGS__);printf("\n");}
#define LOGW(...) {printf(__VA_ARGS__);printf("\n");}
#elif FLAME_ANDROID
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "flame", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "flame", __VA_ARGS__))
#endif

namespace flame
{
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

		KeyCount,

		KeyMax = 0xffffffff
	};

	enum MouseKey
	{
		Mouse_Null = -1,
		Mouse_Left,
		Mouse_Right,
		Mouse_Middle,

		MouseKey_count
	};

	enum DragAndDrop
	{
		DragStart,
		DragEnd,
		DragOvering,
		Dropped
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

	inline bool is_mouse_clicked(KeyStateFlags action, MouseKey key, bool db = false)
	{
		return action == (KeyStateDown | KeyStateUp | (db ? KeyStateDouble : 0)) && key == Mouse_Null;
	}

	inline ulonglong get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

	template <class T = void>
	struct Mail
	{
		T* p;
		void* dtor;
		uint udt_name_hash;

		Mail() :
			p(nullptr),
			dtor(nullptr),
			udt_name_hash(0)
		{
		}

		operator Mail<void>()
		{
			Mail<void> ret;
			ret.p = p;
			ret.dtor = dtor;
			ret.udt_name_hash = udt_name_hash;
			return ret;
		}
	};

	template <class T>
	Mail<T> new_mail(const T* v = nullptr, uint udt_name_hash = 0)
	{
		auto p = f_malloc(sizeof(T));
		if (v)
			new(p) T(*v);
		else
			new(p) T;

		Mail<T> ret;
		ret.p = (T*)p;
		ret.dtor = df2v<T>();
		ret.udt_name_hash = udt_name_hash;

		return ret;
	}

	inline Mail<void*> new_mail_p(void* p)
	{
		return new_mail(&p);
	}

	template <class T>
	void delete_mail(const Mail<T>& m)
	{
		if (!m.p)
			return;
		if (m.dtor)
			cmf(p2f<MF_v_v>(m.dtor), m.p);
		f_free(m.p);
	}

	template <class F>
	struct Closure
	{
		F* function;
		Mail<> capture;

		template <class FF = F, class ...Args>
		auto call(Args... args)
		{
			return ((FF*)function)(capture.p, args...);
		}

		~Closure()
		{
			delete_mail(capture);
			capture.p = nullptr;
		}
	};

	struct ListenerHubImpl
	{
		FLAME_FOUNDATION_EXPORTS static ListenerHubImpl *create();
		FLAME_FOUNDATION_EXPORTS static void destroy(ListenerHubImpl* h);
		FLAME_FOUNDATION_EXPORTS uint count();
		FLAME_FOUNDATION_EXPORTS Closure<void(void*)>& item(uint idx);
		FLAME_FOUNDATION_EXPORTS void* add_plain(void(*pf)(void* c), const Mail<>& capture);
		FLAME_FOUNDATION_EXPORTS void remove_plain(void* c);
	};

	template <class F>
	struct ListenerHub
	{
		ListenerHubImpl* impl;

		void* add(F* pf, const Mail<>& capture)
		{
			return impl->add_plain((void(*)(void* c))pf, capture);
		}

		void remove(void* c)
		{
			impl->remove_plain(c);
		}

		template <class ...Args>
		void call(Args... args)
		{
			auto count = impl->count();
			for (auto i = 0; i < count; i++)
				impl->item(i).call<F>(args...);
		}
	};

	FLAME_FOUNDATION_EXPORTS void* get_hinst();
	FLAME_FOUNDATION_EXPORTS Vec2u get_screen_size();
	FLAME_FOUNDATION_EXPORTS StringW get_curr_path();
	FLAME_FOUNDATION_EXPORTS StringW get_app_path();
	FLAME_FOUNDATION_EXPORTS void set_curr_path(const wchar_t* p);
	FLAME_FOUNDATION_EXPORTS void read_process_memory(void* process, void* address, uint size, void* dst);
	FLAME_FOUNDATION_EXPORTS void sleep(int time); // a time less than 0 means forever
	FLAME_FOUNDATION_EXPORTS void* create_event(bool signaled);
	FLAME_FOUNDATION_EXPORTS void set_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void reset_event(void* ev);
	FLAME_FOUNDATION_EXPORTS bool wait_event(void* ev, int timeout);
	FLAME_FOUNDATION_EXPORTS void destroy_event(void* ev);
	FLAME_FOUNDATION_EXPORTS void do_simple_dispatch_loop();
	FLAME_FOUNDATION_EXPORTS bool is_file_occupied(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void exec(const wchar_t* filename, wchar_t* parameters, bool wait, bool show = false);
	FLAME_FOUNDATION_EXPORTS StringA exec_and_get_output(const wchar_t* filename, wchar_t* parameters);
	FLAME_FOUNDATION_EXPORTS void exec_and_redirect_to_std_output(const wchar_t* filename, wchar_t* parameters);

	FLAME_FOUNDATION_EXPORTS void* get_module_from_address(void* addr);
	FLAME_FOUNDATION_EXPORTS StringW get_module_name(void* module);

	FLAME_FOUNDATION_EXPORTS StringW get_clipboard();
	FLAME_FOUNDATION_EXPORTS void set_clipboard(const wchar_t* s);

	FLAME_FOUNDATION_EXPORTS void open_explorer_and_select(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void move_to_trashbin(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void get_thumbnail(uint width, const wchar_t* filename, uint* out_width, uint* out_height, char** out_data);

	FLAME_FOUNDATION_EXPORTS Key vk_code_to_key(int vkCode);
	FLAME_FOUNDATION_EXPORTS bool is_modifier_pressing(Key k /* accept: Key_Shift, Key_Ctrl and Key_Alt */, int left_or_right /* 0 or 1 */);

	FLAME_FOUNDATION_EXPORTS void* add_global_key_listener(Key key, bool modifier_shift, bool modifier_ctrl, bool modifier_alt, void (*callback)(void* c, KeyStateFlags action), const Mail<>& capture);
	FLAME_FOUNDATION_EXPORTS void remove_global_key_listener(void* handle/* return by add_global_key_listener */);

	enum FileChangeType
	{
		FileAdded,
		FileRemoved,
		FileModified,
		FileRenamed
	};

	FLAME_FOUNDATION_EXPORTS void* /* event */ add_file_watcher(const wchar_t* path, void (*callback)(void* c, FileChangeType type, const wchar_t* filename), const Mail<>& capture, bool all_changes = true, bool sync = true);
	// set_event to returned ev to end the file watching

	FLAME_FOUNDATION_EXPORTS void add_work(void (*function)(void* c), const Mail<>& capture);
	FLAME_FOUNDATION_EXPORTS void clear_all_works();
	FLAME_FOUNDATION_EXPORTS void wait_all_works();

	enum WindowStyle
	{
		WindowFrame = 1 << 0,
		WindowResizable = 1 << 1,
		WindowFullscreen = 1 << 2
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

		CursorCount
	};

	struct SysWindow;
	typedef SysWindow* SysWindowPtr;

	struct SysWindow : Object
	{
		Vec2i pos;
		Vec2u size;
		int style;

		bool minimized;

		SysWindow() :
			Object("SysWindow")
		{
		}

		FLAME_FOUNDATION_EXPORTS void* get_native();

		FLAME_FOUNDATION_EXPORTS const char* title();
		FLAME_FOUNDATION_EXPORTS const void set_title(const char* title);

#ifdef FLAME_WINDOWS
		FLAME_FOUNDATION_EXPORTS void set_cursor(CursorType type);

		FLAME_FOUNDATION_EXPORTS void set_pos(const Vec2i& pos);
		FLAME_FOUNDATION_EXPORTS void set_size(const Vec2i& pos, const Vec2u& size, int style);
		FLAME_FOUNDATION_EXPORTS void set_maximized(bool v);
#endif

		ListenerHub<void(void* c, KeyStateFlags action, int value)>							key_listeners;
		ListenerHub<void(void* c, KeyStateFlags action, MouseKey key, const Vec2i & pos)>	mouse_listeners;
		ListenerHub<void(void* c, const Vec2u & size)>										resize_listeners;
		ListenerHub<void(void* c)>															destroy_listeners;

		FLAME_FOUNDATION_EXPORTS void close();

		FLAME_FOUNDATION_EXPORTS static SysWindow* create(const char* title, const Vec2u& size, WindowStyleFlags style);
		FLAME_FOUNDATION_EXPORTS static void destroy(SysWindow* s);
	};

	struct Looper
	{
		uint frame;
		float delta_time; // second
		float total_time; // second

		FLAME_FOUNDATION_EXPORTS int loop(void (*idle_func)(void* c), const Mail<>& capture);

		FLAME_FOUNDATION_EXPORTS void* add_event(void (*event)(void* c), const Mail<>& capture, void (*ending)(void* c) = nullptr, bool repeatly = false, float interval = 0.f, uint id = 0, bool only = false /* if true, only one event of the id can exists in list */);

		FLAME_FOUNDATION_EXPORTS void remove_event(void* ret_by_add);
		FLAME_FOUNDATION_EXPORTS void clear_events(int id = 0); /* id=-1 means all */
		FLAME_FOUNDATION_EXPORTS void process_events();
	};

	FLAME_FOUNDATION_EXPORTS Looper& looper();

	inline void* add_fps_listener(void (*event)(void* c, uint fps), const Mail<>& capture)
	{
		struct Capture
		{
			uint last_frame;
			void (*e)(void* c, uint fps);
			Mail<> c;
		}e;
		e.last_frame = 0;
		e.e = event;
		e.c = capture;
		return looper().add_event([](void* c) {
			auto& capture = *(Capture*)c;
			auto frame = looper().frame;
			capture.e(capture.c.p, frame - capture.last_frame);
			capture.last_frame = frame;
		}, new_mail(&e), [](void* c) {
			delete_mail(((Capture*)c)->c);
		}, true, 1.f);
	}
}
