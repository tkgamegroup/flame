#include <flame/universe/utils/typeinfo.h>

#include "scene_editor.h"

cInspector::cInspector() :
	Component("cInspector")
{
	auto& ui = scene_editor.window->ui;

	ui.next_element_padding = 4.f;
	auto e_page = ui.e_begin_docker_page(L"Inspector").second;
	{
		auto c_layout = ui.c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}

	ui.e_begin_scrollbar(ScrollbarVertical, true);
		e_layout = ui.e_empty();
		{
			ui.c_element()->clip_flags = ClipChildren;
			auto cl = ui.c_layout(LayoutVertical);
			cl->item_padding = 4.f;
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			ui.c_aligner(AlignMinMax, AlignMinMax);
		}
	ui.e_end_scrollbar();

	refresh();
}

cInspector::~cInspector()
{
	scene_editor.inspector = nullptr;
}

struct cComponentTracker : Component
{
	std::unordered_map<uint, cDataTracker*> vs;

	Component* t;
	void* l;

	cComponentTracker(Component* t) :
		t(t),
		Component("cComponentTracker")
	{
		l = t->data_changed_listeners.add([](Capture& c, uint hash, void* sender) {
			if (sender == scene_editor.inspector)
				return true;
			auto thiz = c.thiz<cComponentTracker>();
			auto it = thiz->vs.find(hash);
			if (it != thiz->vs.end())
				it->second->update_view();
			return true;
		}, Capture().set_thiz(this));
		t->entity->on_destroyed_listeners.add([](Capture& c) {
			c.thiz<cComponentTracker>()->t = nullptr;
			return true;
		}, Capture().set_thiz(this));
	}

	~cComponentTracker()
	{
		if (t)
			t->data_changed_listeners.remove(l);
	}
};

void cInspector::refresh()
{
	auto& ui = scene_editor.window->ui;

	e_layout->remove_children(0, -1);

	ui.parents.push(e_layout);
	if (!scene_editor.selected)
		ui.e_text(L"Nothing Selected");
	else
	{
		ui.e_begin_layout(LayoutHorizontal, 2.f);
		ui.e_text(L"name");
		ui.e_edit(100.f, s2w(scene_editor.selected->name.str()).c_str(), true, true)->get_component(cText)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
			if (hash == FLAME_CHASH("text"))
			{
				auto text = c.current<cText>()->text.v;
				scene_editor.selected->name = w2s(text);
				if (scene_editor.hierarchy)
				{
					auto item = scene_editor.hierarchy->find_item(scene_editor.selected);
					if (item->get_component(cTreeNode))
						item->child(0)->get_component(cText)->set_text(text);
					else
						item->get_component(cText)->set_text(text);
				}
			}
			return true;
		}, Capture());
		ui.e_end_layout();

		ui.e_begin_layout(LayoutHorizontal, 2.f);
		ui.e_text(L"visible");
		ui.e_checkbox(scene_editor.selected->visible_)->get_component(cCheckbox)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
			if (hash == FLAME_CHASH("checked"))
				scene_editor.selected->set_visible(c.current<cCheckbox>()->checked);
			return true;
		}, Capture());
		ui.e_end_layout();

		auto components = scene_editor.selected->get_components();
		for (auto i = 0; i < components.s; i++)
		{
			auto component = components.v[i];

			auto udt = find_udt(FLAME_HASH((std::string("flame::") + component->name).c_str()));
			auto library = udt->db->library;

			ui.e_begin_layout(LayoutHorizontal, 2.f);
			ui.e_text(s2w(component->name).c_str())->get_component(cText)->color = Vec4c(30, 40, 160, 255);
			ui.push_style(ButtonColorNormal, common(Vec4c(0)));
			ui.push_style(ButtonColorHovering, common(ui.style(FrameColorHovering).c));
			ui.push_style(ButtonColorActive, common(ui.style(FrameColorActive).c));
			ui.e_button(L"X", [](Capture& c) {
				looper().add_event([](Capture& c) {
					auto component = c.thiz<Component>();
					scene_editor.inspector->refresh();
					component->entity->remove_component(component);
				}, Capture().set_thiz(c._thiz));
			}, Capture().set_thiz(component));
			ui.pop_style(ButtonColorNormal);
			ui.pop_style(ButtonColorHovering);
			ui.pop_style(ButtonColorActive);
			ui.e_end_layout();

			ui.next_element_padding = 4.f;
			auto esp = ui.e_begin_splitter(SplitterHorizontal);
			esp->get_component(cAligner)->y_align_flags = (AlignFlag)0;
			esp->get_component(cLayout)->height_fit_children = true;
			auto c_component_tracker = new cComponentTracker(component);
			c_component_tracker->id = FLAME_HASH(component->name);
			ui.current_entity->add_component(c_component_tracker);
			auto e_name_list = ui.e_begin_layout(LayoutVertical, 2.f, false, true);
			ui.c_aligner(AlignMinMax, 0);
			ui.e_end_layout();
			auto e_value_list = ui.e_begin_layout(LayoutVertical, 2.f, false, true);
			ui.c_aligner(AlignMinMax, 0);
			ui.e_end_layout();
			ui.e_end_splitter();

			for (auto v : udt->variables)
			{
				auto type = v->type;
				auto base_name = type->base_name.str();
				auto base_hash = type->base_hash;
				auto pdata = (char*)component + v->offset;

				auto f_set = udt->find_function("set_" + v->name.str());
				auto f_set_addr = f_set ? (char*)library + (uint)f_set->rva : nullptr;

				cDataTracker* tracker = nullptr;

				ui.parents.push(e_name_list);
				ui.next_element_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
				auto e_name = ui.e_text(s2w(v->name.str()).c_str());
				ui.c_aligner(AlignMinMax, 0);
				ui.parents.pop();

				switch (type->tag)
				{
				case TypeEnumSingle:
				{
					cCombobox* combobox;

					ui.parents.push(e_value_list);
					auto info = find_enum(base_hash);
					combobox = ui.e_begin_combobox()->get_component(cCombobox);
					for (auto i : info->items)
						ui.e_combobox_item(s2w(i->name.str()).c_str());
					ui.e_end_combobox();
					ui.parents.pop();

					e_name->add_component(f_new<cEnumSingleDataTracker>(pdata, info, [](Capture& c, int v) {
						;
					}, Capture(), combobox));
				}
					break;
				case TypeEnumMulti:
				{
					std::vector<cCheckbox*> checkboxes;

					ui.parents.push(e_value_list);
					ui.next_element_size = Vec2f(8.f, 4.f + ui.style(FontSize).u[0]);
					ui.e_element();
					ui.c_aligner(AlignMinMax, 0);
					ui.parents.pop();

					auto info = find_enum(base_hash);
					for (auto i : info->items)
					{
						ui.parents.push(e_name_list);
						ui.next_element_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
						ui.e_text(s2w(i->name.str()).c_str());
						ui.c_aligner(AlignMinMax, 0);
						ui.parents.pop();

						ui.parents.push(e_value_list);
						checkboxes.push_back(ui.e_checkbox()->get_component(cCheckbox));
						ui.parents.pop();
					}

					e_name->add_component(f_new<cEnumMultiDataTracker>(pdata, info, [](Capture& c, int v) {
						;
					}, Capture(), checkboxes));
				}
					break;
				case TypeData:
				{
					struct Capturing
					{
						void* p;
						void* o;
						void* f;
					}capture;
					capture.p = pdata;
					capture.o = component;
					capture.f = f_set_addr;
					switch (base_hash)
					{
					case FLAME_CHASH("bool"):
					{
						cCheckbox* checkbox;

						ui.parents.push(e_value_list);
						checkbox = ui.e_checkbox()->get_component(cCheckbox);
						ui.c_aligner(AlignMinMax, 0);
						ui.parents.pop();

						e_name->add_component(f_new<cBoolDataTracker>(pdata, [](Capture& c, bool v) {
							;
						}, Capture(), checkbox));
					}
						break;
					case FLAME_CHASH("int"):
					{
						cText* edit_text;
						cText* drag_text;

						ui.parents.push(e_value_list);
						auto e = ui.e_drag_edit();
						e->get_component(cLayout)->width_fit_children = false;
						ui.current_entity = e;
						ui.c_aligner(AlignMinMax, 0);
						ui.current_entity = e->child(0);
						ui.c_aligner(AlignMinMax, 0);
						edit_text = ui.current_entity->get_component(cText);
						ui.current_entity = e->child(1);
						ui.c_aligner(AlignMinMax, 0);
						drag_text = ui.current_entity->get_component(cText);
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalDataTracker<int>>(pdata, [](Capture& c, int v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<int>::set_s(capture.o, capture.f, v, scene_editor.inspector);
							else
								*(int*)capture.p = v;
						}, Capture().set_data(&capture), edit_text, drag_text));
					}
						break;
					case FLAME_CHASH("flame::Vec(2+int)"):
					{
						std::array<cText*, 2> edit_texts;
						std::array<cText*, 2> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 2; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<2, int>>(pdata, [](Capture& c, const Vec<2, int>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<2, int>>::set_s(capture.o, capture.f, (Vec<2, int>*) & v, scene_editor.inspector);
							else
								*(Vec<2, int>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(3+int)"):
					{
						std::array<cText*, 3> edit_texts;
						std::array<cText*, 3> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 3; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<3, int>>(pdata, [](Capture& c, const Vec<3, int>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<3, int>>::set_s(capture.o, capture.f, (Vec<3, int>*) & v, scene_editor.inspector);
							else
								*(Vec<3, int>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(4+int)"):
					{
						std::array<cText*, 4> edit_texts;
						std::array<cText*, 4> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 4; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<4, int>>(pdata, [](Capture& c, const Vec<4, int>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<4, int>>::set_s(capture.o, capture.f, (Vec<4, int>*) & v, scene_editor.inspector);
							else
								*(Vec<4, int>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("uint"):
					{
						cText* edit_text;
						cText* drag_text;

						ui.parents.push(e_value_list);
						auto e = ui.e_drag_edit();
						e->get_component(cLayout)->width_fit_children = false;
						ui.current_entity = e;
						ui.c_aligner(AlignMinMax, 0);
						ui.current_entity = e->child(0);
						ui.c_aligner(AlignMinMax, 0);
						edit_text = ui.current_entity->get_component(cText);
						ui.current_entity = e->child(1);
						ui.c_aligner(AlignMinMax, 0);
						drag_text = ui.current_entity->get_component(cText);
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalDataTracker<uint>>(pdata, [](Capture& c, uint v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<uint>::set_s(capture.o, capture.f, v, scene_editor.inspector);
							else
								*(uint*)capture.p = v;
						}, Capture().set_data(&capture), edit_text, drag_text));
					}
						break;
					case FLAME_CHASH("flame::Vec(2+uint)"):
					{
						std::array<cText*, 2> edit_texts;
						std::array<cText*, 2> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 2; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<2, uint>>(pdata, [](Capture& c, const Vec<2, uint>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<2, uint>>::set_s(capture.o, capture.f, (Vec<2, uint>*) & v, scene_editor.inspector);
							else
								*(Vec<2, uint>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(3+uint)"):
					{
						std::array<cText*, 3> edit_texts;
						std::array<cText*, 3> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 3; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<3, uint>>(pdata, [](Capture& c, const Vec<3, uint>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<3, uint>>::set_s(capture.o, capture.f, (Vec<3, uint>*) & v, scene_editor.inspector);
							else
								*(Vec<3, uint>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(4+uint)"):
					{
						std::array<cText*, 4> edit_texts;
						std::array<cText*, 4> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 4; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<4, uint>>(pdata, [](Capture& c, const Vec<4, uint>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<4, uint>>::set_s(capture.o, capture.f, (Vec<4, uint>*) & v, scene_editor.inspector);
							else
								*(Vec<4, uint>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("float"):
					{
						cText* edit_text;
						cText* drag_text;

						ui.parents.push(e_value_list);
						auto e = ui.e_drag_edit();
						e->get_component(cLayout)->width_fit_children = false;
						ui.current_entity = e;
						ui.c_aligner(AlignMinMax, 0);
						ui.current_entity = e->child(0);
						ui.c_aligner(AlignMinMax, 0);
						edit_text = ui.current_entity->get_component(cText);
						ui.current_entity = e->child(1);
						ui.c_aligner(AlignMinMax, 0);
						drag_text = ui.current_entity->get_component(cText);
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalDataTracker<float>>(pdata, [](Capture& c, float v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<float>::set_s(capture.o, capture.f, v, scene_editor.inspector);
							else
								*(float*)capture.p = v;
						}, Capture().set_data(&capture), edit_text, drag_text));
					}
						break;
					case FLAME_CHASH("flame::Vec(2+float)"):
					{
						std::array<cText*, 2> edit_texts;
						std::array<cText*, 2> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 2; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<2, float>>(pdata, [](Capture& c, const Vec<2, float>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<2, float>>::set_s(capture.o, capture.f, (Vec<2, float>*) & v, scene_editor.inspector);
							else
								*(Vec<2, float>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(3+float)"):
					{
						std::array<cText*, 3> edit_texts;
						std::array<cText*, 3> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 3; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<3, float>>(pdata, [](Capture& c, const Vec<3, float>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<3, float>>::set_s(capture.o, capture.f, (Vec<3, float>*) & v, scene_editor.inspector);
							else
								*(Vec<3, float>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(4+float)"):
					{
						std::array<cText*, 4> edit_texts;
						std::array<cText*, 4> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 4; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<4, float>>(pdata, [](Capture& c, const Vec<4, float>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<4, float>>::set_s(capture.o, capture.f, (Vec<4, float>*) & v, scene_editor.inspector);
							else
								*(Vec<4, float>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("uchar"):
					{
						cText* edit_text;
						cText* drag_text;

						ui.parents.push(e_value_list);
						auto e = ui.e_drag_edit();
						e->get_component(cLayout)->width_fit_children = false;
						ui.current_entity = e;
						ui.c_aligner(AlignMinMax, 0);
						ui.current_entity = e->child(0);
						ui.c_aligner(AlignMinMax, 0);
						edit_text = ui.current_entity->get_component(cText);
						ui.current_entity = e->child(1);
						ui.c_aligner(AlignMinMax, 0);
						drag_text = ui.current_entity->get_component(cText);
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalDataTracker<uchar>>(pdata, [](Capture& c, uchar v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<uchar>::set_s(capture.o, capture.f, v, scene_editor.inspector);
							else
								*(uchar*)capture.p = v;
						}, Capture().set_data(&capture), edit_text, drag_text));
					}
						break;
					case FLAME_CHASH("flame::Vec(2+uchar)"):
					{
						std::array<cText*, 2> edit_texts;
						std::array<cText*, 2> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 2; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<2, uchar>>(pdata, [](Capture& c, const Vec<2, uchar>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<2, uchar>>::set_s(capture.o, capture.f, (Vec<2, uchar>*) & v, scene_editor.inspector);
							else
								*(Vec<2, uchar>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(3+uchar)"):
					{
						std::array<cText*, 3> edit_texts;
						std::array<cText*, 3> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 3; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<3, uchar>>(pdata, [](Capture& c, const Vec<3, uchar>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<3, uchar>>::set_s(capture.o, capture.f, (Vec<3, uchar>*) & v, scene_editor.inspector);
							else
								*(Vec<3, uchar>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::Vec(4+uchar)"):
					{
						std::array<cText*, 4> edit_texts;
						std::array<cText*, 4> drag_texts;

						ui.parents.push(e_value_list);
						ui.e_begin_layout(LayoutHorizontal, 2.f, false, true);
						ui.c_aligner(AlignMinMax, 0);
						for (auto i = 0; i < 4; i++)
						{
							auto e = ui.e_drag_edit();
							e->get_component(cLayout)->width_fit_children = false;
							ui.current_entity = e;
							ui.c_aligner(AlignMinMax, 0);
							ui.current_entity = e->child(0);
							ui.c_aligner(AlignMinMax, 0);
							edit_texts[i] = ui.current_entity->get_component(cText);
							ui.current_entity = e->child(1);
							ui.c_aligner(AlignMinMax, 0);
							drag_texts[i] = ui.current_entity->get_component(cText);
						}
						ui.e_end_layout();
						ui.parents.pop();

						e_name->add_component(f_new<cDigitalVecDataTracker<4, uchar>>(pdata, [](Capture& c, const Vec<4, uchar>& v, bool exit_editing) {
							auto& capture = c.data<Capturing>();
							if (capture.f)
								Setter_t<Vec<4, uchar>>::set_s(capture.o, capture.f, (Vec<4, uchar>*) & v, scene_editor.inspector);
							else
								*(Vec<4, uchar>*)capture.p = v;
						}, Capture().set_data(&capture), edit_texts, drag_texts));
					}
						break;
					case FLAME_CHASH("flame::StringA"):
					{
						cText* text;

						ui.parents.push(e_value_list);
						text = ui.e_edit(50.f)->get_component(cText);
						ui.c_aligner(AlignMinMax, 0);
						ui.parents.pop();

						e_name->add_component(f_new<cStringADataTracker>(pdata, [](Capture& c, const char* v) {
							;
						}, Capture(), text));
					}
						break;
					case FLAME_CHASH("flame::StringW"):
					{
						cText* text;

						ui.parents.push(e_value_list);
						text = ui.e_edit(50.f)->get_component(cText);
						ui.c_aligner(AlignMinMax, 0);
						ui.parents.pop();

						e_name->add_component(f_new<cStringWDataTracker>(pdata, [](Capture& c, const wchar_t* v) {
							;
						}, Capture(), text));
					}
						break;
					}
				}
					break;
				}

				c_component_tracker->vs[FLAME_HASH(v->name.v)] = tracker;
			}
		}

		ui.e_begin_button_menu(L"Add Component");
		{
			FLAME_SAL(prefix, "Serializer_c");
			std::vector<UdtInfo*> all_udts;
			for (auto db : global_typeinfo_databases)
			{
				for (auto u : db->udts.get_all())
				{
					if (u->name.str().compare(0, prefix.l, prefix.s) == 0)
						all_udts.push_back(u);
				}
			}
			std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
				return a->name.str() < b->name.str();
			});
			for (auto udt : all_udts)
			{
				ui.e_menu_item(s2w(udt->name.v + prefix.l).c_str(), [](Capture& c) {
					looper().add_event([](Capture& c) {
						auto u = c.thiz<UdtInfo>();

						auto dummy = malloc(u->size);
						auto library = u->db->library;
						{
							auto f = u->find_function("ctor");
							if (f && f->parameters.s == 0)
								cmf(p2f<MF_v_v>((char*)library + (uint)f->rva), dummy);
						}
						void* component;
						{
							auto f = u->find_function("create");
							assert(f && check_function(f, "P#flame::Component", {}));
							component = cmf(p2f<MF_vp_v>((char*)library + (uint)f->rva), dummy);
						}
						scene_editor.selected->add_component((Component*)component);
						{
							auto f = u->find_function("dtor");
							if (f)
								cmf(p2f<MF_v_v>((char*)library + (uint)f->rva), dummy);
						}
						free(dummy);

						scene_editor.inspector->refresh();
					}, Capture().set_thiz(c.thiz<UdtInfo>()));
				}, Capture().set_thiz(udt));
			}
		}
		ui.e_end_button_menu();
	}
	ui.parents.pop();
}
