#pragma once

#include <flame/foundation/foundation.h>

#define NOMINMAX
#include <Windows.h>

namespace flame
{
	struct WindowBridge : Window
	{
		void set_title(const char* title) override;
	};

	struct WindowPrivate : WindowBridge
	{
		HWND hWnd = 0;

		Vec2i pos;
		Vec2u size;
		std::string title;
		int style;
		CursorType cursor_type = CursorNone;
		HCURSOR cursors[Cursor_Count];

		std::vector<std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>> key_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>> key_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, char)>>> char_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2i&)>>> mouse_left_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2i&)>>> mouse_left_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2i&)>>> mouse_right_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2i&)>>> mouse_right_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2i&)>>> mouse_middle_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2i&)>>> mouse_middle_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2i&)>>> mouse_move_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, int)>>> mouse_scroll_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const Vec2u&)>>> resize_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&)>>> destroy_listeners;

		bool sizing = false;
		Vec2u pending_size;

		bool dead = false;

		WindowPrivate(const std::string& _title, const Vec2u& _size, uint _style, WindowPrivate* parent);
		~WindowPrivate();

		void wnd_proc(UINT message, WPARAM wParam, LPARAM lParam);

		void* get_native() override;

		Vec2i get_pos() const override { return pos; }
		void set_pos(const Vec2i& pos) override;
		Vec2u get_size() const override { return size; }
		void set_size(const Vec2u& size) override;

		const char* get_title() const override { return title.c_str(); }
		void set_title(const std::string& _title);

		int get_style() const override { return style; }

		CursorType get_cursor() override { return cursor_type; }
		void set_cursor(CursorType type) override;

		void close() override;

		void* add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) override;
		void remove_key_down_listener(void* lis) override;
		void* add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) override;
		void remove_key_up_listener(void* lis) override;
		void* add_char_listener(void (*callback)(Capture& c, char ch), const Capture& capture) override;
		void remove_char_listener(void* lis) override;
		void* add_mouse_left_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_left_down_listener(void* lis) override;
		void* add_mouse_left_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_left_up_listener(void* lis) override;
		void* add_mouse_right_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_right_down_listener(void* lis) override;
		void* add_mouse_right_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_right_up_listener(void* lis) override;
		void* add_mouse_middle_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_middle_down_listener(void* lis) override;
		void* add_mouse_middle_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_middle_up_listener(void* lis) override;
		void* add_mouse_move_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_move_listener(void* lis) override;
		void* add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture) override;
		void remove_mouse_scroll_listener(void* lis) override;
		void* add_resize_listener(void (*callback)(Capture& c, const Vec2u& size), const Capture& capture) override;
		void remove_resize_listener(void* lis) override;
		void* add_destroy_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_destroy_listener(void* lis) override;
	};

	inline void WindowBridge::set_title(const char* _title)
	{
		((WindowPrivate*)this)->set_title(_title);
	}

	struct LooperPrivate : Looper
	{
		uint frame = 0;
		float delta_time = 0.f;
		float total_time = 0.f;

		std::vector<std::unique_ptr<WindowPrivate>> windows;
		void (*frame_callback)(Capture& c, float delta_time) = nullptr;
		Capture frame_capture = {};

		uint64 last_time = 0;

		uint get_frame() const override { return frame; }
		float get_delta_time() const override { return delta_time; }
		float get_total_time() const override { return total_time; }

		bool one_frame();

		int loop(void (*frame_callback)(Capture& c, float delta_time), const Capture& capture) override;

		void* add_event(void (*callback)(Capture& c), const Capture& capture, CountDown interval = CountDown(), uint id = 0) override;
		void reset_event(void* ev) override;
		void remove_event(void* ev) override;
		void remove_all_events(int id) override;
	};

	extern LooperPrivate _looper;

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

		bool once;
		std::vector<std::unique_ptr<Item>> items;
		Group* curr_group = nullptr;

		SchedulePrivate(bool once) :
			once(once)
		{
		}

		void release() override { delete this; }

		void add_event(float delay, float duration, void(*callback)(Capture& c, float time, float duration), const Capture& capture) override;
		void begin_group() override;
		void end_group() override;
		void start() override;
		void stop() override;
	};
}
