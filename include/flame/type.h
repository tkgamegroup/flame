#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>

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
	typedef void*				voidptr;

	const auto INVALID_POINTER = (void*)0x7fffffffffffffff;

	template <class T>
	constexpr uint array_size(const T& a)
	{
		return sizeof(a) / sizeof(a[0]);
	}

	template <uint N>
	struct EnsureConstU
	{
		static const uint value = N;
	};

	template <class CH>
	struct _SAL // str and len
	{
		uint l;
		const CH* s;

		_SAL(uint l, const CH* s) :
			l(l),
			s(s)
		{
		}
	};

#define FLAME_SAL_S(x) EnsureConstU<__f_strlen(x)>::value, x
#define FLAME_SAL(n, x) _SAL n(FLAME_SAL_S(x))

	template <class CH>
	constexpr uint __f_strlen(const CH* str)
	{
		auto p = str;
		while (*p)
			p++;
		return p - str;
	}

	inline constexpr uint hash_update(uint h, uint v)
	{
		return h ^ (v + 0x9e3779b9 + (h << 6) + (h >> 2));
	}

	template <class CH>
	constexpr uint hash_str(const CH* str, uint seed)
	{
		return 0 == *str ? seed : hash_str(str + 1, hash_update(seed, *str));
	}

#define FLAME_HASH(x) (hash_str(x, 0))

#define FLAME_CHASH(x) (EnsureConstU<hash_str(x, 0)>::value)

	template <class F>
	void* f2v(F f) // function to void pointer
	{
		union
		{
			F f;
			void* p;
		}cvt;
		cvt.f = f;
		return cvt.p;
	}

	template <class F>
	F p2f(void* p) // void pointer to function
	{
		union
		{
			void* p;
			F f;
		}cvt;
		cvt.p = p;
		return cvt.f;
	}

	typedef void* (*F_vp_v)();

	template <class F, class ...Args>
	auto cf(F f, Args... args) // call function
	{
		return (*f)(args...);
	}

	struct __Dummy__
	{
	};
	typedef void(__Dummy__::* MF_v_v)();
	typedef void(__Dummy__::* MF_v_vp)(void*);
	typedef void(__Dummy__::* MF_v_u)(uint);
	typedef void(__Dummy__::* MF_v_vp_u)(void*, uint);
	typedef void(__Dummy__::* MF_v_b_vp)(bool, void*);
	typedef void(__Dummy__::* MF_v_i_vp)(int, void*);
	typedef void(__Dummy__::* MF_v_u_vp)(uint, void*);
	typedef void(__Dummy__::* MF_v_f_vp)(float, void*);
	typedef void(__Dummy__::* MF_v_c_vp)(uchar, void*);
	typedef void* (__Dummy__::* MF_vp_v)();
	typedef void* (__Dummy__::* MF_vp_vp)(void*);
	typedef bool(__Dummy__::* MF_b_v)();

	template <class F, class ...Args>
	auto cmf(F f, void* p, Args... args) // call member function at an address
	{
		return (*((__Dummy__*)p).*f)(args...);
	}

	inline bool equal_and_not_null(void* a, void* b)
	{
		return a && a == b;
	}

	template <class T>
	std::pair<T*, uchar> find_not_null_and_only(T* a, T* b)
	{
		if (a && !b)
			return std::make_pair(a, 0);
		if (!a && b)
			return std::make_pair(b, 1);
		return std::make_pair(nullptr, 0);
	}

	template <class T>
	bool is_one_of(T v, const std::initializer_list<T>& l)
	{
		for (auto& i : l)
		{
			if (v == i)
				return true;
		}
		return false;
	}
}
