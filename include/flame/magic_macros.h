#pragma once

#define EXPAND(...) __VA_ARGS__

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)

#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X) 
#define FE_2(WHAT, X, ...) WHAT(X)EXPAND(FE_1(WHAT, __VA_ARGS__))
#define FE_3(WHAT, X, ...) WHAT(X)EXPAND(FE_2(WHAT, __VA_ARGS__))
#define FE_4(WHAT, X, ...) WHAT(X)EXPAND(FE_3(WHAT, __VA_ARGS__))
#define FE_5(WHAT, X, ...) WHAT(X)EXPAND(FE_4(WHAT, __VA_ARGS__))
#define FE_6(WHAT, X, ...) WHAT(X)EXPAND(FE_5(WHAT, __VA_ARGS__))
#define FE_7(WHAT, X, ...) WHAT(X)EXPAND(FE_6(WHAT, __VA_ARGS__))
#define FE_8(WHAT, X, ...) WHAT(X)EXPAND(FE_7(WHAT, __VA_ARGS__))

#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME 
#define FOR_EACH(action,...) EXPAND(GET_MACRO(_0,__VA_ARGS__,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__))
