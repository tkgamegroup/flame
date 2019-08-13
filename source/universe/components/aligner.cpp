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

			min_width = -1.f;
			min_height = -1.f;
			width_policy = SizeFixed;
			height_policy = SizeFixed;
		}

		void update()
		{
			if (min_width < 0.f && width_policy == SizeGreedy)
				min_width = element->width;
			if (min_height < 0.f && height_policy == SizeGreedy)
				min_height = element->height;
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
		((cAlignerPrivate*)this)->update();
	}

	cAligner* cAligner::create(Entity* e)
	{
		return new cAlignerPrivate(e);
	}
}
