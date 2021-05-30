namespace flame
{
	struct UI
	{
		inline Entity* e_begin_docker_floating_container()
		{
			auto e = e_empty();
			cDockerTab::make_floating_container(current_root->world, e, next_element_pos, next_element_size);
			next_element_pos = next_element_size = 0.f;
			parents.push(e);
			return e;
		}

		inline Entity* e_begin_docker_static_container()
		{
			auto e = e_empty();
			cDockerTab::make_static_container(current_root->world, e);
			parents.push(e);
			return e;
		}

		inline Entity* e_begin_docker_layout(LayoutType type)
		{
			auto e = e_empty();
			cDockerTab::make_layout(current_root->world, e, type);
			parents.push(e);
			return e;
		}

		inline Entity* e_begin_docker()
		{
			auto e = e_empty();
			cDockerTab::make_docker(current_root->world, e);
			parents.push(e);
			return e;
		}

		inline std::pair<Entity*, Entity*> e_begin_docker_page(const wchar_t* title, void(*on_close)(Capture& c) = nullptr, const Capture& _close_capture = Capture())
		{
			parents.push(parents.top()->children[0]);
			auto et = e_empty();
			et->name = "docker_tab";
			c_element()->padding = Vec4f(4.f, 2.f, style(FontSize).u.x() + 6.f, 2.f);
			c_text()->set_text(title);
			c_event_receiver();
			auto csb = c_style_color2();
			csb->color_normal[0] = style(TabColorNormal).c;
			csb->color_hovering[0] = style(TabColorElse).c;
			csb->color_active[0] = style(TabColorElse).c;
			csb->color_normal[1] = style(SelectedTabColorNormal).c;
			csb->color_hovering[1] = style(SelectedTabColorElse).c;
			csb->color_active[1] = style(SelectedTabColorElse).c;
			csb->style();
			auto cst = c_style_text_color2();
			cst->color_normal[0] = style(TabTextColorNormal).c;
			cst->color_else[0] = style(TabTextColorElse).c;
			cst->color_normal[1] = style(SelectedTabTextColorNormal).c;
			cst->color_else[1] = style(SelectedTabTextColorElse).c;
			cst->style();
			c_list_item();
			c_layout();
			auto cdt = c_docker_tab();
			cdt->root = current_root;
			parents.push(et);
			{
				struct Capturing
				{
					cDockerTab* t;
					void(*f)(Capture&);
				}capture;
				capture.t = cdt;
				capture.f = on_close;
				push_style(TextColorNormal, common(style(TabTextColorElse).c));
				e_button(Icon_TIMES, [](Capture& c) {
					auto& capture = c.data<Capturing>();
					if (capture.f)
						capture.f(c.release<Capturing>());
					looper().add_event([](Capture& c) {
						c.data<cDockerTab*>()->take_away(true);
					}, Capture().set_data(&capture.t));
				}, Capture().absorb(&capture, _close_capture, true), false);
				c_aligner(AlignMax | AlignAbsolute, 0);
				pop_style(TextColorNormal);
			}
			parents.pop();
			parents.pop();
			parents.push(parents.top()->children[1]);
			auto ep = e_empty();
			{
				auto ce = c_element();
				ce->color = style(BackgroundColor).c;
				ce->clip_flags = ClipChildren;
				c_aligner(AlignMinMax, AlignMinMax);
			}
			parents.pop();
			parents.push(ep);
			return std::make_pair(et, ep);
		}
	};

	struct cEnumSingleDataTracker : cDataTracker
	{
		cCombobox* combobox;

		EnumInfo* info;
		void(*on_changed)(Capture& c, int v);
		Capture capture;

		cEnumSingleDataTracker(void* _data, EnumInfo* info, void(*on_changed)(Capture& c, int v), const Capture& capture, cCombobox* combobox) :
			info(info),
			on_changed(on_changed),
			capture(capture),
			combobox(combobox)
		{
			data = _data;

			update_view();

			combobox->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("index"))
				{
					auto thiz = c.thiz<cEnumSingleDataTracker>();
					if (!thiz->updating)
						thiz->on_changed(thiz->capture, thiz->info->items[thiz->combobox->index]->value);
				}
				return true;
			}, Capture().set_thiz(this));
		}

		~cEnumSingleDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			auto idx = -1;
			info->find_item(*(int*)data, &idx);
			updating = true;
			combobox->set_index(idx);
			updating = false;
		}
	};

	struct cEnumMultiDataTracker : cDataTracker
	{
		std::vector<cCheckbox*> checkboxes;

		EnumInfo* info;
		void(*on_changed)(Capture& c, int v);
		Capture capture;

		cEnumMultiDataTracker(void* _data, EnumInfo* info, void(*on_changed)(Capture& c, int v), const Capture& capture, const std::vector<cCheckbox*>& checkboxes) :
			info(info),
			on_changed(on_changed),
			capture(capture),
			checkboxes(checkboxes)
		{
			data = _data;
			assert(checkboxes.size() == info->items.s);

			update_view();

			for (auto i = 0; i < checkboxes.size(); i++)
			{
				checkboxes[i]->data_changed_listeners.add([](Capture& c, uint hash, void*) {
					if (hash == FLAME_CHASH("checked"))
					{
						auto thiz = c.thiz<cEnumMultiDataTracker>();
						if (!thiz->updating)
						{
							auto idx = c.data<int>();
							auto v = *(int*)thiz->data;
							auto f = thiz->info->items[idx]->value;
							if (thiz->checkboxes[idx]->checked)
								v |= f;
							else
								v &= ~f;
							thiz->on_changed(thiz->capture, v);
						}
					}
					return true;
				}, Capture().set_data(&i).set_thiz(this));
			}
		}

		~cEnumMultiDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			updating = true;
			for (auto i = 0; i < checkboxes.size(); i++)
				checkboxes[i]->set_checked(*(int*)data & info->items[i]->value);
			updating = false;
		}
	};

	struct cStringADataTracker : cDataTracker
	{
		cText* text;

		void(*on_changed)(Capture& c, const char* v);
		Capture capture;

		cStringADataTracker(void* _data, void(*on_changed)(Capture& c, const char* v), const Capture& capture, cText* text) :
			on_changed(on_changed),
			capture(capture),
			text(text)
		{
			data = _data;

			update_view();

			auto e_e = text->entity->get_component(cEdit);
			e_e->enter_to_throw_focus = true;
			e_e->trigger_changed_on_lost_focus = true;

			text->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("text"))
				{
					auto thiz = c.thiz<cStringADataTracker>();
					if (!thiz->updating)
						thiz->on_changed(thiz->capture, w2s(c.current<cText>()->text.str()).c_str());
				}
				return true;
			}, Capture().set_thiz(this));
		}

		~cStringADataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			updating = true;
			text->set_text(((StringA*)data)->v ? s2w(((StringA*)data)->str()).c_str() : L"", -1);
			updating = false;
		}
	};

	struct cStringWDataTracker : cDataTracker
	{
		cText* text;

		void(*on_changed)(Capture& c, const wchar_t* v);
		Capture capture;

		cStringWDataTracker(void* _data, void(*on_changed)(Capture& c, const wchar_t* v), const Capture& capture, cText* text) :
			on_changed(on_changed),
			capture(capture),
			text(text)
		{
			data = _data;

			update_view();

			auto e_e = text->entity->get_component(cEdit);
			e_e->enter_to_throw_focus = true;
			e_e->trigger_changed_on_lost_focus = true;

			text->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("text"))
				{
					auto thiz = c.thiz<cStringWDataTracker>();
					if (!thiz->updating)
						thiz->on_changed(thiz->capture, c.current<cText>()->text.v);
				}
				return true;
			}, Capture().set_thiz(this));
		}

		~cStringWDataTracker() override
		{
			f_free(capture._data);
		}

		void update_view() override
		{
			updating = true;
			text->set_text(((StringW*)data)->v ? ((StringW*)data)->v : L"", -1);
			updating = false;
		}
	};
}
