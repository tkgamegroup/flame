#include <flame/universe/components/element.h>
#include <flame/universe/components/aligner.h>

namespace flame
{
	struct cAlignerPrivate: cAligner
	{
		cAlignerPrivate()
		{
			element = nullptr;

			x_align = AlignxFree;
			y_align = AlignyFree;
			min_size = -1.f;
			width_policy = SizeFixed;
			width_factor = 1.f;
			height_policy = SizeFixed;
			height_factor = 1.f;
			using_padding_in_free_layout = false;
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
		}

		void update()
		{
			if (min_size.x() < 0.f && width_policy == SizeGreedy)
				min_size.x() = element->size.x();
			if (min_size.y() < 0.f && height_policy == SizeGreedy)
				min_size.y() = element->size.y();
		}

		Component* copy()
		{
			auto copy = new cAlignerPrivate();

			copy->x_align = x_align;
			copy->y_align = y_align;
			copy->min_size = min_size;
			copy->width_policy = width_policy;
			copy->width_factor = width_factor;
			copy->height_policy = height_policy;
			copy->height_factor = height_factor;
			copy->using_padding_in_free_layout = using_padding_in_free_layout;

			return copy;
		}
	};

	void cAligner::start()
	{
		((cAlignerPrivate*)this)->start();
	}

	void cAligner::update()
	{
		((cAlignerPrivate*)this)->update();
	}

	Component* cAligner::copy()
	{
		return ((cAlignerPrivate*)this)->copy();
	}

	cAligner* cAligner::create()
	{
		return new cAlignerPrivate();
	}
}
