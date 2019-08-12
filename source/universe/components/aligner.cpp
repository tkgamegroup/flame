#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>

namespace flame
{
	struct cAlignerPrivate: cAligner
	{
		cAlignerPrivate(Entity* e) :
			cAligner(e)
		{
			element = (cElement*)(e->find_component(cH("Element")));
			assert(element);

			x_align = AlignxFree;
			y_align = AlignyFree;

			width_greedy = false;
			height_greedy = false;
		}
	};

	cAligner::cAligner(Entity* e) :
		Component("Aligner", e)
	{
	}

	cAligner::~cAligner()
	{
	}

	void cAligner::update()
	{
	}

	cAligner* cAligner::create(Entity* e)
	{
		return new cAlignerPrivate(e);
	}
}
