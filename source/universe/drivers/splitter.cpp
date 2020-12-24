#include "../entity_private.h"
#include "../components/element_private.h"
#include "../components/receiver_private.h"
#include "splitter_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	void cSplitterPrivate::on_gain_bar_receiver()
	{
		//bar_state_listener = bar_receiver->entity->add_local_message_listener([](Capture& c, Message msg, void*) {
		//	auto thiz = c.thiz<cSplitterPrivate>();
		//	switch (msg)
		//	{
		//	case MessageStateChanged:
		//		thiz->bar_receiver->dispatcher->window->set_cursor(
		//			(thiz->bar_receiver->entity->state & StateHovering) ?
		//			(thiz->layout->type == LayoutHorizontal ? CursorSizeWE : CursorSizeNS) : CursorArrow);
		//		break;
		//	}
		//}, Capture().set_thiz(this));
		//bar_mouse_listener = bar_receiver->add_mouse_move_listener([](Capture& c, const ivec2& disp, const ivec2& pos) {
		//	auto thiz = c.thiz<cSplitterPrivate>();
		//	if (thiz->bar_receiver == thiz->bar_receiver->dispatcher->active)
		//	{
		//		auto e = thiz->entity;
		//		if (e->children.size() >= 3 && thiz->bar_receiver->entity->index == 1)
		//		{
		//			auto left = e->children[0].get();
		//			auto left_element = left->get_component_t<cElementPrivate>();
		//			auto left_aligner = left->get_component_t<cAlignerPrivate>();
		//			auto right = e->children[2].get();
		//			auto right_element = right->get_component_t<cElementPrivate>();
		//			auto right_aligner = right->get_component_t<cAlignerPrivate>();
		//			if (left_element && right_element)
		//			{
		//				if (thiz->layout->type == LayoutHorizontal)
		//				{
		//					if (disp.x < 0.f)
		//					{
		//						auto v = min(left_element->size.x - max(1.f, left_aligner ? left_aligner->desired_size.x : left_element->padding_size[0]), (float)-disp.x);
		//						left_element->set_width(left_element->size.x - v);
		//						right_element->set_width(right_element->size.x + v);
		//					}
		//					else if (disp.x > 0.f)
		//					{
		//						auto v = min(right_element->size.x - max(1.f, right_aligner ? right_aligner->desired_size.x : right_element->padding_size[0]), (float)disp.x);
		//						left_element->set_width(left_element->size.x + v);
		//						right_element->set_width(right_element->size.x - v);
		//					}
		//					if (left_aligner)
		//						left_aligner->set_width_factor(left_element->size.x);
		//					if (right_aligner)
		//						right_aligner->set_width_factor(right_element->size.x);
		//				}
		//				else
		//				{
		//					if (disp.y < 0.f)
		//					{
		//						auto v = min(left_element->size.y - max(1.f, left_aligner ? left_aligner->desired_size.y : left_element->padding_size[1]), (float)-disp.y);
		//						left_element->set_height(left_element->size.y - v);
		//						right_element->set_height(right_element->size.y + v);
		//					}
		//					else if (disp.y > 0.f)
		//					{
		//						auto v = min(right_element->size.y - max(1.f, right_aligner ? right_aligner->desired_size.y : right_element->padding_size[1]), (float)disp.y);
		//						left_element->set_height(left_element->size.y + v);
		//						right_element->set_height(right_element->size.y - v);
		//					}
		//					if (left_aligner)
		//						left_aligner->set_height_factor(left_element->size.y);
		//					if (right_aligner)
		//						right_aligner->set_height_factor(right_element->size.y);
		//				}
		//			}
		//		}
		//	}
		//}, Capture().set_thiz(this));
	}

	void cSplitterPrivate::on_lost_bar_receiver()
	{
		//bar_receiver->entity->remove_local_message_listener(bar_state_listener);
		bar_state_listener = nullptr;
	}

	//void cSplitterPrivate::on_child_message(Message msg, void* p)
	//{
	//	switch (msg)
	//	{
	//	case MessageAdded:
	//		if (entity->children.size() == 2)
	//			entity->reposition_child(0, 1);
	//		break;
	//	}
	//}

	dSplitter* dSplitter::create()
	{
		return f_new<cSplitterPrivate>();
	}
}
