#pragma once

#include <stdarg.h>
#include <assert.h>
#include <string>
#include <array>
#include <vector>
#include <span>
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
	typedef long long			int64;
	typedef unsigned long long  uint64;
	typedef void*				voidptr;

	const auto INVALID_POINTER = (void*)0x7fffffffffffffff;

	template<class T, size_t N>
	constexpr size_t size(T(&)[N]) { return N; }

	template <class T>
	void* var_end(T* p)
	{
		return (char*)p + sizeof(T);
	}

	template <auto V>
	struct S
	{
		constexpr static decltype(V) v = V;
	};

	uint64 constexpr ch(char const* str)
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

	template <class T>
	void erase_if(std::vector<T>& vec, T v)
	{
		auto it = std::find(vec.begin(), vec.end(), v);
		if (it != vec.end())
			vec.erase(it);
	}

	template <class T>
	void erase_if(std::vector<std::unique_ptr<T>>& vec, T* v)
	{
		auto it = std::find_if(vec.begin(), vec.end(), [&](const auto& t) {
			return t.get() == v;
		});
		if (it != vec.end())
			vec.erase(it);
	}

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

	template <class F, class ...Args>
	auto cf(F f, Args... args) // call function
	{
		return (*f)(args...);
	}

	struct __Dummy__
	{
	};

	template <class F, class ...Args>
	auto cmf(F f, void* p, Args... args) // call member function at an address
	{
		return (*((__Dummy__*)p).*f)(args...);
	}

	struct CountDown
	{
		bool is_frame;
		union
		{
			uint frames;
			float time;
		}v;

		CountDown() :
			is_frame(true)
		{
			v.frames = 0;
		}

		CountDown(uint frames) :
			is_frame(true)
		{
			v.frames = frames;
		}

		CountDown(float time) :
			is_frame(false)
		{
			v.time = time;
		}
	};
}
