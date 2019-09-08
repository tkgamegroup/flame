namespace flame
{
	namespace ui
	{
		//struct BP_Node_Drawdata : Dtor
		//{
		//	BP_Node_Drawdata(blueprint::Node *_n, Instance *_ui, BP_Scene_Draw_Private *_scene_priv)
		//	{
		//		for (auto i = 0; i < n->inputslot_count(); i++)
		//		{
		//			auto slot = n->inputslot(i);

		//			w_s->add_listener(ListenerDrop, [this, slot](Widget *src) {
		//				if (src->class_hash == cH("SLOT"))
		//				{
		//					auto wslot = (wSlot*)src;
		//					if (wslot->io() == 1)
		//						scene_priv->s->add_link(wslot->slot(), slot);
		//				}
		//			});

		//			switch (slot->type)
		//			{
		//			case blueprint::SlotTypeStr:
		//			{
		//				auto e = wEdit::create(ui);
		//				e->align = AlignLittleEnd;
		//				e->sdf_scale() = 1.f;
		//				e->set_size_by_width(100.f);
		//				lv->add_child(e);

		//				e->set_text(s2w(slot->s_val.data).c_str());

		//				e->add_listener(ListenerChanged, [slot, e]() {
		//					slot->s_val = w2s(e->text()).c_str();
		//				});
		//			}
		//				break;
		//			case blueprint::SlotTypeInt:
		//			{
		//				auto e = wEdit::create(ui);
		//				e->align = AlignLittleEnd;
		//				e->sdf_scale() = 1.f;
		//				e->set_size_by_width(100.f);
		//				e->add_char_filter_int();
		//				lv->add_child(e);

		//				e->set_text(std::to_wstring(slot->i_val[0]).c_str());

		//				e->add_listener(ListenerChanged, [slot, e]() {
		//					slot->i_val[0] = stoi(e->text());
		//				});
		//			}
		//				break;
		//			case blueprint::SlotTypeInt2:
		//			{
		//				auto e0 = wEdit::create(ui);
		//				e0->align = AlignLittleEnd;
		//				e0->sdf_scale() = 1.f;
		//				e0->set_size_by_width(100.f);
		//				e0->add_char_filter_int();
		//				e0->set_text(std::to_wstring(slot->i_val[0]).c_str());
		//				lv->add_child(e0);
		//				auto e1 = wEdit::create(ui);
		//				e1->align = AlignLittleEnd;
		//				e1->sdf_scale() = 1.f;
		//				e1->set_size_by_width(100.f);
		//				e1->add_char_filter_int();
		//				e1->set_text(std::to_wstring(slot->i_val[1]).c_str());
		//				lv->add_child(e1);

		//				e0->add_listener(ListenerChanged, [slot, e0]() {
		//					slot->i_val[0] = stoi(e0->string_storage(0));
		//				});
		//				e1->add_listener(ListenerChanged, [slot, e1]() {
		//					slot->i_val[1] = stoi(e1->text());
		//				});
		//			}
		//				break;
		//			case blueprint::SlotTypeFloat:
		//			{
		//				auto e = wEdit::create(ui);
		//				e->align = AlignLittleEnd;
		//				e->text_col() = Bvec4(255);
		//				e->sdf_scale() = 1.f;
		//				e->set_size_by_width(100.f);
		//				e->add_char_filter_float();
		//				lv->add_child(e);

		//				e->set_text(std::to_wstring(slot->f_val[0]).c_str());

		//				e->add_listener(ListenerChanged, [slot, e]() {
		//					slot->f_val[0] = stof(e->text());
		//				});
		//			}
		//				break;
		//			case blueprint::SlotTypeBool:
		//			{
		//				auto c = wCheckbox::create(ui);
		//				c->align = AlignLittleEnd;
		//				add_style_buttoncolor(c, 0, Vec3(0.f, 0.f, 0.7f));
		//				lv->add_child(c);

		//				c->checked() = slot->b_val;

		//				c->add_listener(ListenerChanged, [slot, c]() {
		//					slot->b_val = c->checked();
		//				});
		//			}
		//				break;
		//			case blueprint::SlotTypeSingle:
		//			{
		//				auto c = wCombo::create(ui);
		//				c->align = AlignLittleEnd;
		//				c->w_btn()->sdf_scale() = 1.f;
		//				add_style_buttoncolor(c, 0, Vec3(0.f, 0.f, 0.7f));

		//				auto e = scene_priv->s->typeinfo_db()->find_cpp_enumeration(H(slot->s_val.data));
		//				for (auto j = 0; j < e->item_count(); j++)
		//				{
		//					auto i = wMenuItem::create(ui, s2w(e->item(j)->name()).c_str());
		//					c->w_items()->add_child(i);
		//					i->background_col.w = 255;
		//					i->sdf_scale() = 1.f;
		//					add_style_buttoncolor(i, 0, Vec3(0.f, 0.f, 0.7f));
		//				}

		//				c->set_sel(e->find_item(slot->i_val[0]));

		//				lv->add_child(c);

		//				c->add_listener(ListenerChanged, [slot, c, e]() {
		//					uint v = e->item(c->sel())->value();
		//					slot->i_val[0] = v;
		//				});
		//			}
		//				break;
		//			case blueprint::SlotTypeMulti:
		//			{
		//				auto e = scene_priv->s->typeinfo_db()->find_cpp_enumeration(H(slot->s_val.data));
		//				for (auto j = 0; j < e->item_count(); j++)
		//				{
		//					auto t = wToggle::create(ui);
		//					t->align = AlignLittleEnd;
		//					t->sdf_scale() = 1.f;
		//					t->set_text_and_size(s2w(e->item(j)->name()).c_str());
		//					add_style_buttoncolor(t, 0, Vec3(0.f, 0.f, 0.7f));
		//					add_style_buttonsize(t, 0, Vec2(2.f, 0.f));
		//					add_style_buttoncolor(t, 1, Vec3(150.f, 1.f, 0.8f));
		//					add_style_buttonsize(t, 1, Vec2(2.f, 0.f));
		//					lv->add_child(t);

		//					uint v = e->item(j)->value();

		//					t->set_toggle((slot->i_val[0] & v) != 0);

		//					t->add_listener(ListenerChanged, [slot, t, v]() {
		//						if (t->toggled())
		//							slot->i_val[0] |= v;
		//						else
		//							slot->i_val[0] &= ~v;
		//					});
		//				}
		//			}
		//				break;
		//			}
		//		}

		//		for (auto i = 0; i < n->outputslot_count(); i++)
		//		{
		//			auto slot = n->outputslot(i);

		//			w_s->add_listener(ListenerDrop, [this, slot](Widget *src) {
		//				if (src->class_hash == cH("SLOT"))
		//				{
		//					auto wslot = (wSlot*)src;
		//					if (wslot->io() == 0)
		//						scene_priv->s->add_link(slot, wslot->slot());
		//				}
		//			});
		//		}
		//	}
		//};

		//inline BP_Scene_Draw_Private::BP_Scene_Draw_Private(Instance *_ui, blueprint::Scene *_s) :
		//	ui(_ui),
		//	s(_s)
		//{
		//	sel = 0;

		//	w_scene->add_draw_command([](CommonData *d) {
		//		auto c = (Canvas*)d[0].p;
		//		auto off = Vec2::from(d[1].f);
		//		auto scl = d[2].f[0];
		//		auto thiz = (BP_Scene_Draw_Private*)d[3].p;

		//		auto dw = (wSlot*)thiz->w_scene->instance()->dragging_widget();
		//		if (dw && dw->class_hash == cH("SLOT"))
		//		{
		//			auto p1 = get_slot_pos(dw);
		//			auto p4 = Vec2(thiz->w_scene->instance()->mouse_pos);
		//			c->add_bezier(p1, p1 +
		//				Vec2(50.f * ((dw->io() == 0) ? -1.f : 1.f), 0.f) * thiz->w_scene->global_scale, p4, p4, Colorf(1.f, 1.f, 0.f), 3.f * thiz->w_scene->global_scale);
		//		}
		//	}, "p", this);

		//	w_scene->add_listener(ListenerLeftMouseDown, [this](const Vec2 &mpos) {
		//		sel = 0;

		//		for (auto i = 0; i < s->link_count(); i++)
		//		{
		//			auto l = s->link(i);

		//			auto w_o = (Widget*)l->out_slot->user_data;
		//			auto w_i = (Widget*)l->in_slot->user_data;
		//			auto po = w_o->global_pos + w_o->size * 0.5f * w_o->global_scale;
		//			auto pi = w_i->global_pos + w_i->size * 0.5f * w_i->global_scale;

		//			auto pos = closet_point_to_bezier(mpos, pi, pi + Vec2(-50.f, 0.f) * w_scene->global_scale,
		//				po + Vec2(50.f, 0.f) * w_scene->global_scale, po, 4, 7);
		//			if (distance(mpos, pos) <= 3.f * w_scene->global_scale)
		//			{
		//				sel = l;
		//				return;
		//			}
		//		}
		//	});
	}
}

