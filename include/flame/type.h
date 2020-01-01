#pragma once

#define FLAME_STR_(x) #x
#define FLAME_STR(x) FLAME_STR_(x)
#define FLAME_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define FLAME_DERIVE_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)
#define FLAME_LOW(I) ((I) & 0xffff)
#define FLAME_HIGH(I) ((I) >> 16)
#define FLAME_MAKEINT(H, L) ((L) | ((H) << 16))
#define FLAME_INVALID_POINTER ((void*)0x7fffffffffffffff)

template <unsigned int N>
struct EnsureConstU
{
	static const unsigned int value = N;
};

template<class CH>
constexpr unsigned int str_len(const CH* str)
{
	auto p = str;
	while (*p)
		p++;
	return p - str;
}

template <class CH>
struct _SAL // str and len
{
	unsigned int l;
	const CH* s;

	_SAL(unsigned int l, const CH* s) :
		l(l),
		s(s)
	{
	}
};

#define SAL_S(x) EnsureConstU<str_len(x)>::value, x
#define SAL(n, x) _SAL n(SAL_S(x))

inline constexpr unsigned int hash_update(unsigned int h, unsigned int v)
{
	return h ^ (v + 0x9e3779b9 + (h << 6) + (h >> 2));
}

template<class CH>
constexpr unsigned int hash_str(const CH* str, unsigned int seed)
{
	return 0 == *str ? seed : hash_str(str + 1, hash_update(seed , *str));
}

#define H(x) (hash_str(x, 0))

#define cH(x) (EnsureConstU<hash_str(x, 0)>::value)

namespace flame
{
	typedef char*				charptr;
	typedef wchar_t*			wcharptr;
	typedef unsigned char		uchar;
	typedef unsigned short		ushort;
	typedef unsigned int		uint;
	typedef unsigned long		ulong;
	typedef long long			longlong;
	typedef unsigned long long  ulonglong;

	typedef void* voidptr;
}
