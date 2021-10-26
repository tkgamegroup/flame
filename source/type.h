#pragma once

#include <stdarg.h>
#include <assert.h>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <span>
#include <stack>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <random>

namespace flame
{
	typedef char*				charptr;
	typedef wchar_t*			wcharptr;
	typedef unsigned char		uchar;
	typedef unsigned short		ushort;
	typedef unsigned int		uint;
	typedef long long			int64;
	typedef unsigned long long  uint64;
	typedef void*				voidptr;

	const auto INVALID_POINTER = (void*)0x7fffffffffffffff;

	template <class T>
	concept long_signed_integral = std::signed_integral<T> && sizeof(T) > sizeof(int);

	template <class T>
	concept long_unsigned_integral = std::unsigned_integral<T> && sizeof(T) > sizeof(uint);

#define FLAME_PTR(name) struct name; typedef name name##T; typedef name* name##Ptr;
#define FLAME_PTR_P(name) struct name##Private; typedef name##Private name##T; typedef name##Private* name##Ptr;

	template <class T>
	void* var_end(T* p)
	{
		return (char*)p + sizeof(T);
	}

	template <auto V> inline constexpr auto S = V;

	template <class T>
	std::span<T> SP(const T& v)
	{
		return std::span(&(T&)v, 1);
	}

	constexpr uint ch(char const* str)
	{
		auto ret = std::_FNV_offset_basis;
		while (*str)
		{
			ret ^= *str;
			ret *= std::_FNV_prime;
			str++;
		}
		return ret;
	}

	constexpr uint operator "" _h(char const* str, std::size_t len)
	{
		return ch(str);
	}

	template <class F>
	void* f2a(F f) // function to address
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
	F a2f(void* p) // address to function
	{
		union
		{
			void* p;
			F f;
		}cvt;
		cvt.p = p;
		return cvt.f;
	}
}
