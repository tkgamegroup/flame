#include <flame/universe/components/list.h>

namespace flame
{
	struct cListPrivate : cList
	{
		void update()
		{
		}
	};

	cList::~cList()
	{
		((cListPrivate*)this)->~cListPrivate();
	}

	void cList::update()
	{
		((cListPrivate*)this)->update();
	}

	cList* cList::create()
	{
		return new cListPrivate();
	}
}
