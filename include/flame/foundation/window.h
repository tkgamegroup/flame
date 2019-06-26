#pragma once

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Application;

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

	struct Window
	{
		Vec2i pos;
		Vec2u size;
		int style;

		bool minimized;

		FLAME_FOUNDATION_EXPORTS void *get_native();

		FLAME_FOUNDATION_EXPORTS const char* title();

#ifdef FLAME_WINDOWS
		FLAME_FOUNDATION_EXPORTS void set_cursor(CursorType type);

		FLAME_FOUNDATION_EXPORTS void set_size(const Vec2i &_pos, const Vec2u &_size, int _style);
		FLAME_FOUNDATION_EXPORTS void set_maximized(bool v);
#endif

		FLAME_FOUNDATION_EXPORTS void add_key_listener(void (*listener)(void* c, KeyState action, Key key), const Mail<>& capture);
		FLAME_FOUNDATION_EXPORTS void add_mouse_listener(void (*listener)(void* c, KeyState action, MouseKey key, const Vec2i& pos), const Mail<>& capture);
		FLAME_FOUNDATION_EXPORTS void add_resize_listener(void (*listener)(void* c, const Vec2u& size), const Mail<>& capture);
		FLAME_FOUNDATION_EXPORTS void add_destroy_listener(void (*listener)(void* c), const Mail<>& capture);

		FLAME_FOUNDATION_EXPORTS void remove_key_listener(void (*listener)(void* c, KeyState action, Key key), const Mail<>& capture);
		FLAME_FOUNDATION_EXPORTS void remove_mouse_listener(void (*listener)(void* c, KeyState action, MouseKey key, const Vec2i& pos), const Mail<>& capture);
		FLAME_FOUNDATION_EXPORTS void remove_resize_listener(void (*listener)(void* c, const Vec2u& size), const Mail<>& capture);
		FLAME_FOUNDATION_EXPORTS void remove_destroy_listener(void (*listener)(void* c), const Mail<>& capture);

		FLAME_FOUNDATION_EXPORTS void close();

		FLAME_FOUNDATION_EXPORTS static Window *create(Application *app, const char *_title, const Vec2u& _size, int _style);
		FLAME_FOUNDATION_EXPORTS static void destroy(Window *s);
	};

	struct Application
	{
		long long total_frame;
		long long fps;
		float elapsed_time; // second

		FLAME_FOUNDATION_EXPORTS int run(void (*idle_func)(void* c), const Mail<>& capture);

		FLAME_FOUNDATION_EXPORTS void clear_delay_events();
		FLAME_FOUNDATION_EXPORTS void add_delay_event(void (*event)(void* c), const Mail<>& capture);

		FLAME_FOUNDATION_EXPORTS static Application *create();
		FLAME_FOUNDATION_EXPORTS static void destroy(Application *m);
	};
}
