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

		void release() override;

		uint count() const override;
		Closure<bool(Capture&)>& item(uint idx) const override;
		void* add(bool(*pf)(Capture& c), const Capture& capture, int pos) override;
		void remove(void* l) override;
	};

	struct WindowPrivate : Window
	{
#ifdef FLAME_WINDOWS
		HWND hWnd;
#elif FLAME_ANDROID
		android_app* android_state;
#endif

		Vec2i pos;
		Vec2u size;
		std::string title;
		int style;
		CursorType cursor_type;
#ifdef FLAME_WINDOWS
		HCURSOR cursors[Cursor_Count];
#endif

		bool sizing;
		Vec2u pending_size;

		bool dead;

#ifdef FLAME_WINDOWS
		WindowPrivate(const std::string& _title, const Vec2u& _size, uint _style, WindowPrivate* parent);
#elif FLAME_ANDROID
		WindowPrivate(android_app* android_state);
#endif
		~WindowPrivate();

		void release() override;

		void* get_native() override;

		Vec2i get_pos() const override;
		void set_pos(const Vec2i& pos) override;
		Vec2u get_size() const override;
		void set_size(const Vec2u& size) override;

		const char* get_title() const override;
		void set_title(const char* title) override;

		int get_style() const override;

		CursorType get_cursor() override;
		void set_cursor(CursorType type) override;

		void close() override;
	};
}
