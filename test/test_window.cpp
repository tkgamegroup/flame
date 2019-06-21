#include <flame/foundation/window.h>

using namespace flame;

template<class F>
struct StupidFunction
{
	void* f;

	StupidFunction(F _f)
	{
		f = _f;
	}

	template<class ...Args>
	auto operator()(Args... args)
	{
		union
		{
			F f;
			void* p;
		}cvt;
		cvt.p = f;
		return cvt.f(args...);
	}
};

template<class T>
struct D
{
	T t;

	void fuck()
	{
		t.~T();
	}
};

// TODO: use enable_if and is_pod

int main(int argc, char** args)
{
	auto app = Application::create();
	auto w = Window::create(app, "Window Test", Vec2u(1280, 720), WindowFrame);

	auto s = StupidFunction(+[]() {});
	s();

	std::string a = "1";

	auto f = &D<std::string>::fuck;
	(*((D<std::string>*)&a).*f)();

	w->add_mouse_listener(Function<void(void*, KeyState, MouseKey, const Vec2i&)>(
		[](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			if (is_mouse_down(action, key))
				(*((WindowPtr*)c))->close();
		}, sizeof(void*), &w));

	app->run(Function<void(void* c)>(
		[](void* c) {
		}));

	return 0;
}
