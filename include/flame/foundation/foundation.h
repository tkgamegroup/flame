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

#include <stack>
#include <list>
#include <map>
#include <unordered_map>
#include <chrono>
#include <fstream>
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

	inline bool is_key_down(KeyState action) // value is Key
	{
		return action == KeyStateDown;
	}

	inline bool is_key_up(KeyState action) // value is Key
	{
		return action == KeyStateUp;
	}

	inline bool is_key_char(KeyState action) // value is ch
	{
		return action == KeyStateNull;
	}

	inline bool is_mouse_enter(KeyState action, MouseKey key)
	{
		return action == KeyStateDown && key == Mouse_Null;
	}

	inline bool is_mouse_leave(KeyState action, MouseKey key)
	{
		return action == KeyStateUp && key == Mouse_Null;
	}

	inline bool is_mouse_down(KeyState action, MouseKey key, bool just = false) // value is pos
	{
		return action == (KeyStateDown | (just ? KeyStateJust : 0)) && key != Mouse_Null;
	}

	inline bool is_mouse_up(KeyState action, MouseKey key, bool just = false) // value is pos
	{
		return action == (KeyStateUp | (just ? KeyStateJust : 0)) && key != Mouse_Null;
	}

	inline bool is_mouse_move(KeyState action, MouseKey key) // value is disp
	{
		return action == KeyStateNull && key == Mouse_Null;
	}

	inline bool is_mouse_scroll(KeyState action, MouseKey key) // value.x() is scroll value
	{
		return action == KeyStateNull && key == Mouse_Middle;
	}

	inline bool is_mouse_clicked(KeyState action, MouseKey key, bool db = false)
	{
		return action == (KeyStateDown | KeyStateUp | (db ? KeyStateDouble : 0)) && key == Mouse_Null;
	}

	inline ulonglong get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	}

#ifdef FLAME_WINDOWS
	inline std::string translate(const char* src_locale, const char* dst_locale, const std::string& src)
	{
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>>
			cv1(new std::codecvt_byname<wchar_t, char, mbstate_t>(src_locale));
		std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>>
			cv2(new std::codecvt_byname<wchar_t, char, mbstate_t>(dst_locale));
		return cv2.to_bytes(cv1.from_bytes(src));
	}

	inline std::string japanese_to_chinese(const std::string& src)
	{
		return translate(".932", ".936", src);
	}
#endif

	template<class T>
	inline T read(std::ifstream& file)
	{
		T v;
		file.read((char*)& v, sizeof(T));
		return v;
	}

	inline std::string read_string(std::ifstream& file)
	{
		uint size = 0;
		uint q = 1;
		for (auto i = 0; i < 4; i++)
		{
			unsigned char byte;
			file.read((char*)& byte, 1);
			if (byte >= 128)
				byte -= 128;
			else
				i = 4;
			size += q * byte;
			q *= 128;
		}
		std::string v;
		v.resize(size);
		file.read((char*)v.data(), size);
		return v;
	}

	template<class T>
	inline void write(std::ofstream& file, const T& v)
	{
		file.write((char*)& v, sizeof(T));
	}

	inline void write_string(std::ofstream& file, const std::string& v)
	{
		uint size = v.size();
		for (auto i = 0; i < 4; i++)
		{
			unsigned char byte = size % 128;
			size /= 128;
			if (size > 0)
				byte += 128;
			else
				i = 4;
			file.write((char*)& byte, 1);

		}
		file.write((char*)v.data(), v.size());
	}

	enum FileType
	{
		FileTypeUnknown,
		FileTypeFolder,
		FileTypeText,
		FileTypeImage,
		FileTypeModel
	};

	inline bool is_text_file(const std::wstring& _ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".txt" ||
			ext == L".h" || ext == L".c" || ext == L".cpp" || ext == L".hpp" || ext == L".cxx" || ext == L".inl" ||
			ext == L".glsl" || ext == L".vert" || ext == L".tesc" || ext == L".tese" || ext == L".geom" || ext == L".frag" || ext == L".hlsl" ||
			ext == L".xml" || ext == L".json" || ext == L".ini" || ext == L".log" ||
			ext == L".htm" || ext == L".html" || ext == L".css" ||
			ext == L".sln" || ext == L".vcxproj")
			return true;
		return false;
	}

	inline bool is_image_file(const std::wstring& _ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".bmp" || ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".gif" ||
			ext == L".tga" || ext == L".dds" || ext == L".ktx")
			return true;
		return false;
	}

	inline bool is_model_file(const std::wstring& _ext)
	{
		auto ext = _ext;
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext == L".obj" || ext == L".pmd" || ext == L".pmx" || ext == L".dae")
			return true;
		return false;
	}

	inline FileType get_file_type(const std::wstring& ext)
	{
		if (is_text_file(ext))
			return FileTypeText;
		if (is_image_file(ext))
			return FileTypeImage;
		if (is_model_file(ext))
			return FileTypeModel;
		return FileTypeUnknown;
	}

	inline longlong get_file_length(std::ifstream& f)
	{
		f.seekg(0, std::ios::end);
		auto s = f.tellg();
		f.seekg(0, std::ios::beg);
		return s;
	}

	inline std::pair<std::unique_ptr<char[]>, longlong> get_file_content(const std::wstring& filename)
	{
#ifdef FLAME_WINDOWS
		std::ifstream file(filename, std::ios::binary);
#else
		auto utf8_filename = w2s(filename);
		std::ifstream file(utf8_filename, std::ios::binary);
#endif
		if (!file.good())
			return std::make_pair(nullptr, 0);

		auto length = get_file_length(file);
		auto data = new char[length + 1];
		file.read(data, length);
		data[length] = 0;
		return std::make_pair(std::unique_ptr<char[]>(data), length);
	}

	inline std::string get_file_string(const std::wstring& filename)
	{
		auto content = get_file_content(filename);
		if (content.first)
			return std::string(content.first.get(), content.first.get() + content.second);
		return std::string();
	}

	template<class T = void>
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

	template<class T>
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

	template<class T>
	void delete_mail(const Mail<T>& m)
	{
		if (!m.p)
			return;
		if (m.dtor)
			cmf(p2f<MF_v_v>(m.dtor), m.p);
		f_free(m.p);
	}

	template<class F>
	struct Closure
	{
		F* function;
		Mail<> capture;

		template<class FF = F, class ...Args>
		auto call(Args... args)
		{
			return ((FF*)function)(capture.p, args...);
		}

		~Closure()
		{
			delete_mail(capture);
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

	template<class F>
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

		template<class ...Args>
		void call(Args... args)
		{
			auto count = impl->count();
			for (auto i = 0; i < count; i++)
				impl->item(i).call<F>(args...);
		}
	};

	struct Object
	{
		const char* name;
		const uint name_hash;
		uint id;

		Object(const char* name) :
			name(name),
			name_hash(H(name)),
			id(0)
		{
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
	FLAME_FOUNDATION_EXPORTS StringA compile_to_dll(uint source_count, const wchar_t* const* sources, uint library_count, const wchar_t* const* libraries, const wchar_t* out);

	FLAME_FOUNDATION_EXPORTS Array<StringA> get_module_dependancies(const wchar_t* module_name);
	FLAME_FOUNDATION_EXPORTS void* get_module_from_address(void* addr);
	FLAME_FOUNDATION_EXPORTS StringW get_module_name(void* module);
	FLAME_FOUNDATION_EXPORTS void* load_module(const wchar_t* module_name);
	FLAME_FOUNDATION_EXPORTS void* get_module_func(void* module, const char* name);
	FLAME_FOUNDATION_EXPORTS void free_module(void* library);

	FLAME_FOUNDATION_EXPORTS StringW get_clipboard();
	FLAME_FOUNDATION_EXPORTS void set_clipboard(const wchar_t* s);

	FLAME_FOUNDATION_EXPORTS void open_explorer_and_select(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void move_to_trashbin(const wchar_t* filename);
	FLAME_FOUNDATION_EXPORTS void get_thumbnail(uint width, const wchar_t* filename, uint* out_width, uint* out_height, char** out_data);

	FLAME_FOUNDATION_EXPORTS Key vk_code_to_key(int vkCode);
	FLAME_FOUNDATION_EXPORTS bool is_modifier_pressing(Key k /* accept: Key_Shift, Key_Ctrl and Key_Alt */, int left_or_right /* 0 or 1 */);

	FLAME_FOUNDATION_EXPORTS void* add_global_key_listener(Key key, bool modifier_shift, bool modifier_ctrl, bool modifier_alt, void (*callback)(void* c, KeyState action), const Mail<>& capture);
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

	struct Window;
	typedef Window* WindowPtr;

	struct Window : Object
	{
		Vec2i pos;
		Vec2u size;
		int style;

		bool minimized;

		Window() :
			Object("Window")
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

		ListenerHub<void(void* c, KeyState action, int value)>							key_listeners;
		ListenerHub<void(void* c, KeyState action, MouseKey key, const Vec2i & pos)>	mouse_listeners;
		ListenerHub<void(void* c, const Vec2u & size)>									resize_listeners;
		ListenerHub<void(void* c)>														destroy_listeners;

		FLAME_FOUNDATION_EXPORTS void close();

		FLAME_FOUNDATION_EXPORTS static Window* create(const char* title, const Vec2u& size, uint style);
		FLAME_FOUNDATION_EXPORTS static void destroy(Window* s);
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
