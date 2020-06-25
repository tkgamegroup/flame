#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	void* _allocate(uint size);
	void _deallocate(void* p);

	struct Delecter
	{
		template <class T>
		void operator()(T* p) 
		{
			p->~T();
			_deallocate(p);
		}
	};
}
