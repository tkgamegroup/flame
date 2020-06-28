#pragma once

#include <flame/foundation/foundation.h>

#ifdef FLAME_WINDOWS
#define NOMINMAX
#include <Windows.h>
#elif FLAME_ANDROID
#include <android_native_app_glue.h>
#endif

namespace flame
{
	struct ListenerHubImnplPrivate : ListenerHubImpl
	{
		std::vector<std::unique_ptr<Closure<bool(Capture& c)>>> listeners;

		void release() override { delete this; }

		uint count() const override { return listeners.size(); }
		Closure<bool(Capture&)>& item(uint idx) const override { return *listeners[idx].get(); }
		void* add(bool(*pf)(Capture& c), const Capture& capture, int pos) override;
		void remove(void* l) override;
	};

	struct WindowPrivate : Window
	{
#ifdef FLAME_WINDOWS
		HWND _hWnd = 0;
#elif FLAME_ANDROID
		android_app* _android_state;
#endif

		Vec2i _pos;
		Vec2u _size;
		std::string _title;
		int _style;
		CursorType _cursor_type = CursorNone;
#ifdef FLAME_WINDOWS
		HCURSOR _cursors[Cursor_Count];
#endif

		std::vector<std::unique_ptr<Closure<void(Capture&, KeyStateFlags, int)>>> _key_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, KeyStateFlags, MouseKey, const Vec2i&)>>> _mouse_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2u&)>>> _resize_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&)>>> _destroy_listeners;

		bool _sizing = false;
		Vec2u _pending_size;

		bool _dead = false;

#ifdef FLAME_WINDOWS
		WindowPrivate(const std::string& _title, const Vec2u& _size, uint _style, WindowPrivate* parent);
#elif FLAME_ANDROID
		WindowPrivate(android_app* android_state);
#endif
		~WindowPrivate();

#ifdef FLAME_WINDOWS
		void _wnd_proc(UINT message, WPARAM wParam, LPARAM lParam);
#endif
		void* _get_native();
		void _set_pos(const Vec2i& pos);
		void _set_size(const Vec2u& size);
		void _set_title(const std::string& title);
		void _set_cursor(CursorType type);
		void _close();

		void* get_native() override { return _get_native(); }

		Vec2i get_pos() const override { return _pos; }
		void set_pos(const Vec2i& pos) override { _set_pos(pos); }
		Vec2u get_size() const override { return _size; }
		void set_size(const Vec2u& size) override { _set_size(size); }

		const char* get_title() const override { return _title.c_str(); }
		void set_title(const char* title) override { _set_title(title); }

		int get_style() const override { return _style; }

		CursorType get_cursor() override { return _cursor_type; }
		void set_cursor(CursorType type) override { _set_cursor(type); }

		void close() override {_close(); }

		void* add_key_listener(void (*callback)(Capture& c, KeyStateFlags action, int value), const Capture& capture) override;
		void remove_key_listener(void* ret) override;
		void* add_mouse_listener(void (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_listener(void* ret) override;
		void* add_resize_listener(void (*callback)(Capture& c, const Vec2u& size), const Capture& capture) override;
		void remove_resize_listener(void* ret) override;
		void* add_destroy_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_destroy_listener(void* ret) override;
	};

	struct LooperPrivate : Looper
	{
		uint _frame = 0;
		float _delta_time = 0.f;
		float _total_time = 0.f;

		std::vector<std::unique_ptr<WindowPrivate>> windows;
		void (*_frame_callback)(Capture& c, float delta_time) = nullptr;
		Capture _frame_capture = {};

		uint64 _last_time = 0;

		bool _one_frame();
		int _loop(void (*frame_callback)(Capture& c, float delta_time), const Capture& capture);

		void* _add_event(void (*callback)(Capture& c), const Capture& capture, CountDown interval = CountDown(), uint id = 0);
		void _reset_event(void* ev);
		void _remove_event(void* ev);
		void _remove_events(int id);
		void _process_events();

		uint get_frame() const override { return _frame; }
		float get_delta_time() const override { return _delta_time; }
		float get_total_time() const override { return _total_time; }

		int loop(void (*frame_callback)(Capture& c, float delta_time), const Capture& capture) override { return _loop(frame_callback, capture); }

		void* add_event(void (*callback)(Capture& c), const Capture& capture, CountDown interval, uint id) override { return _add_event(callback, capture, interval, id); }
		void reset_event(void* ev) override { _reset_event(ev); }
		void remove_event(void* ev) override { _remove_event(ev); }
		void remove_events(int id) override { _remove_events(id); }
		void process_events() override { _process_events(); }
	};

	extern LooperPrivate* _looper;

	struct SchedulePrivate : Schedule
	{
		struct Group;

		struct Item
		{
			enum Type
			{
				TypeGroup,
				TypeEvent,
			};

			Type type;
			int index;

			Item(Type type, int index) :
				type(type),
				index(index)
			{
			}

			virtual void excute(SchedulePrivate* s) = 0;
			virtual ~Item() {}
		};

		struct Event : Item
		{
			Group* group;

			float delay;
			float duration;
			float rest;
			void(*callback)(Capture& c, float time, float duration);
			Capture capture;

			Event(Group* _group, int index) :
				Item(Item::TypeEvent, index)
			{
				group = _group;
			}

			~Event() override
			{
				f_free(capture._data);
			}

			void add_to_looper(SchedulePrivate* s);

			void excute(SchedulePrivate* s) override;
		};

		struct Group : Item
		{
			std::vector<std::unique_ptr<Event>> events;
			int complete_count;

			Group(int index) :
				Item(Item::TypeGroup, index)
			{
			}

			void excute(SchedulePrivate* s) override;
		};

		bool _once;
		std::vector<std::unique_ptr<Item>> _items;
		Group* _curr_group = nullptr;

		SchedulePrivate(bool once) :
			_once(once)
		{
		}

		void _add_event(float delay, float duration, void(*callback)(Capture& c, float time, float duration), const Capture& capture);
		void _begin_group();
		void _end_group();
		void _start();
		void _stop();

		void release() override { delete this; }

		void add_event(float delay, float duration, void(*callback)(Capture& c, float time, float duration), const Capture& capture) override { _add_event(delay, duration, callback, capture); }
		void begin_group() override { _begin_group(); }
		void end_group() override { _end_group(); }
		void start() override { _start(); }
		void stop() override { _stop(); }
	};

}
