#pragma once

#include "foundation.h"

#define NOMINMAX
#include <Windows.h>

namespace flame
{
	struct NativeWindowPrivate : NativeWindow
	{
		HWND hWnd = 0;

		ivec2 pos;
		uvec2 size;
		std::wstring title;
		int style;
		CursorType cursor_type = CursorArrow;
		HCURSOR cursors[Cursor_Count];

		std::vector<std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>> key_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, KeyboardKey)>>> key_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, wchar_t)>>> char_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_left_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_left_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_right_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_right_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_middle_down_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_middle_up_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const ivec2&)>>> mouse_move_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, int)>>> mouse_scroll_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&, const uvec2&)>>> resize_listeners;
		std::vector<std::unique_ptr<Closure<void(Capture&)>>> destroy_listeners;

		bool sizing = false;
		uvec2 pending_size;

		bool dead = false;

		NativeWindowPrivate(const std::wstring& _title, const uvec2& _size, uint _style, NativeWindowPrivate* parent);
		~NativeWindowPrivate();

		void release() override;

		LRESULT wnd_proc(UINT message, WPARAM wParam, LPARAM lParam);

		void* get_native() override;

		ivec2 get_pos() const override { return pos; }
		void set_pos(const ivec2& pos) override;
		uvec2 get_size() const override { return size; }
		void set_size(const uvec2& size) override;

		ivec2 global_to_local(const ivec2& p) override;

		const wchar_t* get_title() const override { return title.c_str(); }
		void set_title(const std::wstring& _title);
		void set_title(const wchar_t* title) override { set_title(std::wstring(title)); }

		int get_style() const override { return style; }

		CursorType get_cursor() override { return cursor_type; }
		void set_cursor(CursorType type) override;

		void* add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) override;
		void remove_key_down_listener(void* lis) override;
		void* add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) override;
		void remove_key_up_listener(void* lis) override;
		void* add_char_listener(void (*callback)(Capture& c, wchar_t ch), const Capture& capture) override;
		void remove_char_listener(void* lis) override;
		void* add_mouse_left_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_left_down_listener(void* lis) override;
		void* add_mouse_left_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_left_up_listener(void* lis) override;
		void* add_mouse_right_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_right_down_listener(void* lis) override;
		void* add_mouse_right_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_right_up_listener(void* lis) override;
		void* add_mouse_middle_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_middle_down_listener(void* lis) override;
		void* add_mouse_middle_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_middle_up_listener(void* lis) override;
		void* add_mouse_move_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) override;
		void remove_mouse_move_listener(void* lis) override;
		void* add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture) override;
		void remove_mouse_scroll_listener(void* lis) override;
		void* add_resize_listener(void (*callback)(Capture& c, const uvec2& size), const Capture& capture) override;
		void remove_resize_listener(void* lis) override;
		void* add_destroy_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_destroy_listener(void* lis) override;
	};

	struct LooperPrivate : Looper
	{
		uint frame = 0;
		uint64 last_time = 0;
		float delta_time = 0.f;
		float total_time = 0.f;
		uint fps = 0;
		uint fps_counting = 0;
		float fps_delta = 0.f;

		std::vector<std::unique_ptr<NativeWindowPrivate>> windows;
		void (*frame_callback)(Capture& c, float delta_time) = nullptr;
		Capture frame_capture = {};

		uint get_frame() const override { return frame; }
		float get_delta_time() const override { return delta_time; }
		float get_total_time() const override { return total_time; }
		uint get_fps() const override { return fps; }

		bool one_frame();

		int loop(void (*frame_callback)(Capture& c, float delta_time), const Capture& capture) override;

		void* add_event(void (*callback)(Capture& c), const Capture& capture, CountDown interval = CountDown(), uint id = 0) override;
		void reset_event(void* ev) override;
		void remove_event(void* ev) override;
		void remove_events(int id) override;
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
