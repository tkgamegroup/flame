

#define EXPAND(...) __VA_ARGS__

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)

#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X) 
#define FE_2(WHAT, X, ...) WHAT(X)FE_1(WHAT, __VA_ARGS__)

#define GET_MACRO(_0,_1,_2,NAME,...) NAME 
#define FOR_EACH(action,...) EXPAND(GET_MACRO(_0,__VA_ARGS__,FE_2,FE_1,FE_0)(action,__VA_ARGS__))

#define DECLARE(x) decltype(x) x;
#define SET_TO_CAPTURE(x) CONCATENATE(__capture__, __LINE__).x = x;
#define GET_FROM_CAPTURE(x) auto x = CONCATENATE(__capture__, __LINE__).x;

#define BEGIN_CAPTURE {struct __CAPTURE__{FOR_EACH(DECLARE, ARGS)}CONCATENATE(__capture__, __LINE__);FOR_EACH(SET_TO_CAPTURE, ARGS)
#define END_CAPTURE }
#define EXPAND_CAPTURE auto& CONCATENATE(__capture__, __LINE__) = *(__CAPTURE__*)c; FOR_EACH(GET_FROM_CAPTURE, ARGS)

int main(int argc, char** args)
{
	int a = 100;
	float b = 0.5f;

#define ARGS a, b
	BEGIN_CAPTURE;
	auto lambda = [](void* c) {
		EXPAND_CAPTURE;
	};
	END_CAPTURE;
#undef ARGS

	return 0;
}
