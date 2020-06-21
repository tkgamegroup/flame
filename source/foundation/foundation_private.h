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
		void wnd_proc(UINT message, WPARAM wParam, LPARAM lParam);
#endif

		void release() override;

		void* get_native() override;

		Vec2i get_pos() const override { return _pos; }
		void set_pos(const Vec2i& pos) override;
		Vec2u get_size() const override { return _size; }
		void set_size(const Vec2u& size) override;

		const char* get_title() const override { return _title.c_str(); }
		void set_title(const char* title) override;

		int get_style() const override { return _style; }

		CursorType get_cursor() override { return _cursor_type; }
		void set_cursor(CursorType type) override;

		void close() override { _dead = true; }

		void* add_key_listener(void (*callback)(Capture& c, KeyStateFlags action, int value), const Capture& capture) override;
		void remove_key_listener(void* ret) override;
		void* add_mouse_listener(void (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture) override;
		void remove_mouse_listener(void* ret) override;
		void* add_resize_listener(void (*callback)(Capture& c, const Vec2u& size), const Capture& capture) override;
		void remove_resize_listener(void* ret) override;
		void* add_destroy_listener(void (*callback)(Capture& c), const Capture& capture) override;
		void remove_destroy_listener(void* ret) override;
	};
}
