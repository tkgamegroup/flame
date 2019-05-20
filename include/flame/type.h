// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#define FLAME_STR_(x) #x
#define FLAME_STR(x) FLAME_STR_(x)
#define FLAME_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define FLAME_DERIVE_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)
#define FLAME_LOW(I) ((I) & 0xffff)
#define FLAME_HIGH(I) ((I) >> 16)
#define FLAME_MAKEINT(H, L) ((L) | ((H) << 16))

namespace flame
{
	typedef char*				charptr;
	typedef wchar_t*			wcharptr;
	typedef unsigned char		uchar;
	typedef unsigned short		ushort;
	typedef unsigned int		uint;
	typedef unsigned long		ulong;
	typedef unsigned long long  ulonglong;

	typedef void* voidptr;
}
