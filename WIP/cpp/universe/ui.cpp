namespace flame
{
	inline void UIPrivate::set_hovering_element(Element * w)
	{
		if (w == hovering_element_)
			return;
		if (hovering_element_)
			hovering_element_->on_mouse(KeyStateUp, Mouse_Null, Vec2(0.f));
		hovering_element_ = w;
		if (hovering_element_)
			hovering_element_->on_mouse(KeyStateDown, Mouse_Null, Vec2(0.f));
	}

	inline void UIPrivate::set_focus_element(Element * w)
	{
		auto old = focus_element_;
		focus_element_ = w;
		if (old)
			old->on_focus(Focus_Lost, 0);
		if (focus_element_)
			focus_element_->on_focus(Focus_Gain, 0);
		set_key_focus_element(w);
	}

	void UIPrivate::show(void* __p, Element * e, bool visible, const Vec2 & off, float scl)
	{
		for (auto i_a = 0; i_a < e->animations$.size; )
		{
			auto& a = e->animations$[i_a];
			a.time += elp_time_;
			a.f$.p.thiz() = &a;
			a.f$.p.e() = e;

			if (a.time >= a.duration$)
			{
				a.time = -1.f;
				a.f$.exec();
				e->animations$.remove(i_a);
			}
			else
			{
				a.f$.exec();
				i_a++;
			}
		}
	}

	//void Drawlist::draw_grid(const Vec2 &wnd_off, const Vec2 &off, const Vec2 &size)
	//{
	//	for (auto i = mod((int)off.x, 100); i.y < size.x; i.y += 100, i.x--)
	//	{
	//		if (i.y < 0)
	//			continue;
	//		add_line(Vec2(i.y, 0.f) + wnd_off, Vec2(i.y, size.y) + wnd_off, Vec4(1.f));
	//		add_text_stroke(Vec2(i.y + 4, 0.f) + wnd_off, Vec4(1.f), "%d", i.x * -100);
	//	}
	//	for (auto i = mod((int)off.y, 100); i.y < size.y; i.y += 100, i.x--)
	//	{
	//		if (i.y < 0)
	//			continue;
	//		add_line(Vec2(0.f, i.y) + wnd_off, Vec2(size.x, i.y) + wnd_off, Vec4(1.f));
	//		add_text_stroke(Vec2(4.f, i.y) + wnd_off, Vec4(1.f), "%d", i.x * -100);
	//	}
	//}
}

