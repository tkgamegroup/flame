//#define EXPAND(...) __VA_ARGS__
//
//#define CONCATENATE_DETAIL(x, y) x##y
//#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
//
//#define FE_0(WHAT)
//#define FE_1(WHAT, X) WHAT(X) 
//#define FE_2(WHAT, X, ...) WHAT(X)EXPAND(FE_1(WHAT, __VA_ARGS__))
//#define FE_3(WHAT, X, ...) WHAT(X)EXPAND(FE_2(WHAT, __VA_ARGS__))
//#define FE_4(WHAT, X, ...) WHAT(X)EXPAND(FE_3(WHAT, __VA_ARGS__))
//#define FE_5(WHAT, X, ...) WHAT(X)EXPAND(FE_4(WHAT, __VA_ARGS__))
//#define FE_6(WHAT, X, ...) WHAT(X)EXPAND(FE_5(WHAT, __VA_ARGS__))
//#define FE_7(WHAT, X, ...) WHAT(X)EXPAND(FE_6(WHAT, __VA_ARGS__))
//#define FE_8(WHAT, X, ...) WHAT(X)EXPAND(FE_7(WHAT, __VA_ARGS__))
//
//#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME 
//#define FOR_EACH(action,...) EXPAND(GET_MACRO(_0,__VA_ARGS__,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__))
//
//#define DECLARE(x) decltype(x) x;
//#define SET_TO_CAPTURE(x) CONCATENATE(__capture__, __LINE__).x = x;
//#define GET_FROM_CAPTURE(x) auto& x = CONCATENATE(__capture__, __LINE__).x;
//
//#define PACK_CAPTURE struct CONCATENATE(__CAPTURE__, FOR_EACH(CONCATENATE, ARGS)){FOR_EACH(DECLARE, ARGS)}CONCATENATE(__capture__, __LINE__);FOR_EACH(SET_TO_CAPTURE, ARGS)
//#define EXPAND_CAPTURE auto& CONCATENATE(__capture__, __LINE__) = *(CONCATENATE(__CAPTURE__, FOR_EACH(CONCATENATE, ARGS))*)pdata; FOR_EACH(GET_FROM_CAPTURE, ARGS)

//	int a = 100;
//	float b = 0.5f;
//	char c = '@';
//	long long d = 0xff;
//
//#define ARGS a, b, c, d
//	PACK_CAPTURE;
//	auto lambda = [](void* pdata) {
//		EXPAND_CAPTURE;
//
//	};
//#undef ARGS

#include <stdio.h>

template <class F>
void* function2address(F f)
{
	union
	{
		F f;
		void* a;
	}c;
	c.f = f;
	return c.a;
}

extern "C" int fun_asm(void* f, void* o);

void fb()
{
	printf("abc\n");
}

struct A
{
	int v;

	void f()
	{
		printf("%d\n", v);
	}
};

int main(int argc, char** args)
{
	A a;
	a.v = 10;
	a.f();

	fun_asm(function2address(&A::f), &a);

	return 0;
}
