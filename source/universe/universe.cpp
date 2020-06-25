#include "universe_private.h"

namespace flame
{
	static void*(*pf_allocate)(Capture& c, uint size);
	static void(*pf_deallocate)(Capture& c, void* p);
	static Capture allocator_capture;

	void* _allocate(uint size)
	{
		return pf_allocate(allocator_capture, size);
	}

	void _deallocate(void* p)
	{
		pf_deallocate(allocator_capture, p);
	}

	void set_allocator(void* (*allocate)(Capture& c, uint size), void(*deallocate)(Capture& c, void* p), const Capture& capture)
	{
		f_free(allocator_capture._data);

		pf_allocate = allocate;
		pf_deallocate = deallocate;
		allocator_capture = capture;
	}
}
