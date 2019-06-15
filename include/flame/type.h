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
