//#include <flame/universe/components/element.h>
#include "../universe_private.h"
#include "aligner_private.h"

namespace flame
{
	//cAlignerPrivate()
	//{
	//	element = nullptr;

	//	margin = Vec4f(0.f);
	//	x_align_flags = (AlignFlag)0;
	//	y_align_flags = (AlignFlag)0;
	//	min_width = -1.f;
	//	min_height = -1.f;
	//	width_factor = 1.f;
	//	height_factor = 1.f;
	//}

	//void on_event(EntityEvent e, void* t) override
	//{
	//	if (e == EntityComponentAdded && t == this)
	//	{
	//		element = entity->get_component(cElement);
	//		assert(element);
	//		if (x_align_flags & AlignGreedy && min_width < 0.f)
	//			min_width = element->size.x();
	//		if (y_align_flags & AlignGreedy && min_height < 0.f)
	//			min_height = element->size.y();
	//	}
	//}
//
//	cAligner* cAligner::create()
//	{
//		return new cAlignerPrivate();
//	}
//
//	void cAligner::set_x_align_flags(uint a, void* sender)
//	{
//		if (a == x_align_flags)
//			return;
//		x_align_flags = (AlignFlag)a;
//		data_changed(FLAME_CHASH("x_align_flags"), sender);
//	}
//
//	void cAligner::set_y_align_flags(uint a, void* sender)
//	{
//		if (a == y_align_flags)
//			return;
//		y_align_flags = (AlignFlag)a;
//		data_changed(FLAME_CHASH("y_align_flags"), sender);
//	}
//
//	void cAligner::set_min_width(float w, void* sender)
//	{
//		if (w == min_width)
//			return;
//		min_width = w;
//		data_changed(FLAME_CHASH("min_width"), sender);
//	}
//
//	void cAligner::set_min_height(float h, void* sender)
//	{
//		if (h == min_height)
//			return;
//		min_height = h;
//		data_changed(FLAME_CHASH("min_height"), sender);
//	}
//
//	void cAligner::set_width_factor(float f, void* sender)
//	{
//		if (f == width_factor)
//			return;
//		width_factor = f;
//		data_changed(FLAME_CHASH("width_factor"), sender);
//	}
//
//	void cAligner::set_height_factor(float f, void* sender)
//	{
//		if (f == height_factor)
//			return;
//		height_factor = f;
//		data_changed(FLAME_CHASH("height_factor"), sender);
//	}

	cAlignerPrivate* cAlignerPrivate::create()
	{
		auto ret = _allocate(sizeof(cAlignerPrivate));
		new (ret) cAlignerPrivate;
		return (cAlignerPrivate*)ret;
	}

	cAligner* cAligner::create() { return cAlignerPrivate::create(); }
}
