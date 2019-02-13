// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/universe.h>

#include <vector>

namespace flame
{
	struct VaribleInfo;
	struct SerializableNode;

	struct UI;

	/*
			   pos                        size.x
				   +------------------------------------------------
				   |	              top inner padding
				   |			****************************
				   |	 left   *                          *  right
			size.y |	 inner  *          content         *  inner
				   |	padding *                          * padding
				   |	        ****************************
				   |			     bottom inner padding
	*/

	enum SizePolicy
	{
		SizeFixed,
		SizeFitChildren,
		SizeFitLayout,
		SizeGreedy
	};

	enum Align
	{
		AlignFree,
		AlignLittleEnd,
		AlignLargeEnd,
		AlignMiddle,
		AlignLeft,
		AlignRight,
		AlignTop,
		AlignBottom,
		AlignLeftTop,
		AlignLeftBottom,
		AlignRightTop,
		AlignRightBottom,
		AlignLeftNoPadding,
		AlignRightNoPadding,
		AlignTopNoPadding,
		AlignBottomNoPadding,
		AlignLeftTopNoPadding,
		AlignLeftBottomNoPadding,
		AlignRightTopNoPadding,
		AlignRightBottomNoPadding,
		AlignCenter,
		AlignLeftOutside,
		AlignRightOutside,
		AlignTopOutside,
		AlignBottomOutside
	};

	enum EventAttitude
	{
		EventAccept,
		EventIgnore,
		EventBlackHole
	};

	enum LayoutType
	{
		LayoutFree,
		LayoutVertical,
		LayoutHorizontal,
		LayoutGrid
	};

	enum State
	{
		StateNormal,
		StateHovering,
		StateActive
	};

	struct Element;
	typedef Element* ElementPtr;

	struct Element : R
	{
		StringAndHash class$;

		String name$;

		Vec2 pos$;
		Vec2 size$;

		float alpha$;
		float scale$;

		Vec4 inner_padding$; // L R T B
		float layout_padding$;

		Vec4 background_offset$; // L T R B
		float background_round_radius$;
		int background_round_flags$;
		float background_frame_thickness$;
		Bvec4 background_col$;
		Bvec4 background_frame_col$;
		float background_shaow_thickness$;

		SizePolicy size_policy_hori$;
		SizePolicy size_policy_vert$;

		Align align$;

		LayoutType layout_type$;
		float item_padding$;
		int grid_hori_count$;
		bool clip$;

		float scroll_offset$;

		EventAttitude event_attitude$;
		bool want_key_focus$;

		bool visible$;

		Vec2 global_pos; // vaild after instance processing
		float global_scale; // vaild after instance processing

		bool cliped; // valid after arranging by parent
		int content_size; // valid after arranging
		bool showed; // vaild after instance processing
		State state; // vaild after instance processing

		Array<Element*> children_1$;
		Array<Element*> children_2$;

		bool draw_default$;
		FLAME_PACKAGE_BEGIN(ExtraDrawParm)
			FLAME_PACKAGE_ITEM(graphics::CanvasPtr, canvas, p)
			FLAME_PACKAGE_ITEM(Vec2, off, f2)
			FLAME_PACKAGE_ITEM(float, scl, f1)

			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
		FLAME_PACKAGE_END
		Array<Function<ExtraDrawParm>> extra_draws$;

		int closet_id$;
		FLAME_PACKAGE_BEGIN(StyleParm)
			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
			FLAME_PACKAGE_ITEM(int, closet_id, i1)
		FLAME_PACKAGE_END
		int style_level;
		Array<Function<StyleParm>> styles$;

		FLAME_PACKAGE_BEGIN(AnimationParm)
			FLAME_PACKAGE_ITEM(float, time, f1)

			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
			FLAME_PACKAGE_ITEM(float, duration, f1)
			FLAME_PACKAGE_ITEM(int, looping, i1)
		FLAME_PACKAGE_END
		Array<Function<AnimationParm>> animations$;

		FLAME_PACKAGE_BEGIN(FoucusListenerParm)
			FLAME_PACKAGE_ITEM(FocusType, type, i1)
			FLAME_PACKAGE_ITEM(int, focus_or_keyfocus, i1)
			
			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
		FLAME_PACKAGE_END

		FLAME_PACKAGE_BEGIN(KeyListenerParm)
		/*
			- when key down/up, action is KeyStateDown or KeyStateUp, value is Key
			- when char, action is KeyStateNull, value is ch
		*/
			FLAME_PACKAGE_ITEM(KeyState, action, i1)
			FLAME_PACKAGE_ITEM(int, value, i1)

			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
		FLAME_PACKAGE_END

		FLAME_PACKAGE_BEGIN(MouseListenerParm)
		/*
			- when enter/leave, action is KeyStateDown or KeyStateUp, key is Mouse_Null
			- when down/up, action is KeyStateDown or KeyStateUp, key is MouseKey, value is pos
			- when move, action is KeyStateNull, key is Mouse_Null, value is disp
			- when scroll, action is KeyStateNull, key is Mouse_Middle, value.x is scroll value
			- when clicked, action is KeyStateDown | KeyStateUp | (KeyStateDouble ? for double clicked), key is Mouse_Null
		*/
			FLAME_PACKAGE_ITEM(KeyState, action, i1)
			FLAME_PACKAGE_ITEM(MouseKey, key, i1)
			FLAME_PACKAGE_ITEM(Vec2, value, f2)

			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
		FLAME_PACKAGE_END

		FLAME_PACKAGE_BEGIN(DropListenerParm)
			FLAME_PACKAGE_ITEM(ElementPtr, src, p)

			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
		FLAME_PACKAGE_END

		FLAME_PACKAGE_BEGIN(ChangedListenerParm)
			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
		FLAME_PACKAGE_END

		enum ChildOp
		{
			ChildAdd,
			ChildRemove
		};
		FLAME_PACKAGE_BEGIN(ChildListenerParm)
			FLAME_PACKAGE_ITEM(ChildOp, op, i1)
			FLAME_PACKAGE_ITEM(ElementPtr, src, p)

			FLAME_PACKAGE_ITEM(ElementPtr, thiz, p)
		FLAME_PACKAGE_END

		Array<Function<FoucusListenerParm>> focus_listeners$;
		Array<Function<KeyListenerParm>> key_listeners$;
		Array<Function<MouseListenerParm>> mouse_listeners$;
		Array<Function<DropListenerParm>> drop_listeners$;
		Array<Function<ChangedListenerParm>> changed_listeners$;
		Array<Function<ChildListenerParm>> child_listeners$;

		CommonData datas$[8];
		StringW text$;

		inline Element()
		{
			class$ = "";

			pos$ = Vec2(0.f);
			size$ = Vec2(0.f);

			alpha$ = 1.f;
			scale$ = 1.f;

			inner_padding$ = Vec4(0.f);
			layout_padding$ = 0.f;

			background_offset$ = Vec4(0.f);
			background_round_radius$ = 0.f;
			background_round_flags$ = 0;
			background_frame_thickness$ = 0.f;
			background_col$ = Bvec4(0);
			background_frame_col$ = Bvec4(255);
			background_shaow_thickness$ = 0.f;

			size_policy_hori$ = SizeFixed;
			size_policy_vert$ = SizeFixed;

			align$ = AlignFree;

			layout_type$ = LayoutFree;
			item_padding$ = 0.f;
			grid_hori_count$ = 1;
			clip$ = false;

			scroll_offset$ = 0.f;

			event_attitude$ = EventAccept;
			want_key_focus$ = false;

			visible$ = true;

			global_pos = Vec2(0.f);
			global_scale = 1.f;

			cliped = false;
			content_size = 0.f;
			showed = false;
			state = StateNormal;

			closet_id$ = 0;

			draw_default$ = true;
		}

		FLAME_UNIVERSE_EXPORTS void set_width(float x, Element* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height(float y, Element* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2& v, Element* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS void set_visibility(bool v);

		FLAME_UNIVERSE_EXPORTS UI* ui() const;
		FLAME_UNIVERSE_EXPORTS Element* parent() const;
		FLAME_UNIVERSE_EXPORTS int layer() const;

		FLAME_UNIVERSE_EXPORTS void add_child(Element* w, int layer = 0, int pos = -1, bool delay = false, bool modual = false);
		FLAME_UNIVERSE_EXPORTS void remove_child(int layer, int idx, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void remove_child(Element* w, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_child(int layer, int idx, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_child(Element* w, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void clear_children(int layer, int begin, int end, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_children(int layer, int begin, int end, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void remove_from_parent(bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_from_parent(bool delay = false);
		FLAME_UNIVERSE_EXPORTS int find_child(Element* w);
		FLAME_UNIVERSE_EXPORTS void set_to_foreground();

		FLAME_UNIVERSE_EXPORTS float get_content_size() const;

		FLAME_UNIVERSE_EXPORTS void arrange();

		FLAME_UNIVERSE_EXPORTS void add_extra_draw(PF pf, const std::vector<CommonData>& capt);

		FLAME_UNIVERSE_EXPORTS void add_style(int closet_id, PF pf, const std::vector<CommonData>& capt, int pos = -1);
		FLAME_UNIVERSE_EXPORTS void remove_style(int idx);

		FLAME_UNIVERSE_EXPORTS void add_animation(float duration, int looping, PF pf, const std::vector<CommonData>& capt);

		FLAME_UNIVERSE_EXPORTS void on_draw(graphics::Canvas* c, const Vec2& off, float scl);
		FLAME_UNIVERSE_EXPORTS void on_focus(FocusType type, int focus_or_keyfocus);
		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, int value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2& value);
		FLAME_UNIVERSE_EXPORTS void on_drop(Element* src);
		FLAME_UNIVERSE_EXPORTS void on_changed();

		FLAME_FOUNDATION_EXPORTS int add_focus_listener(Function<FoucusListenerParm>& listener);
		FLAME_FOUNDATION_EXPORTS int add_key_listener(Function<KeyListenerParm>& listener);
		FLAME_FOUNDATION_EXPORTS int add_mouse_listener(Function<MouseListenerParm>& listener);
		FLAME_FOUNDATION_EXPORTS int add_drop_listener(Function<DropListenerParm>& listener);
		FLAME_FOUNDATION_EXPORTS int add_changed_listener(Function<ChangedListenerParm>& listener);
		FLAME_FOUNDATION_EXPORTS int add_child_listener(Function<ChildListenerParm>& listener);

		FLAME_FOUNDATION_EXPORTS void remove_focus_listener(int idx, bool delay = false);
		FLAME_FOUNDATION_EXPORTS void remove_key_listener(int idx, bool delay = false);
		FLAME_FOUNDATION_EXPORTS void remove_mouse_listener(int idx, bool delay = false);
		FLAME_FOUNDATION_EXPORTS void remove_drop_listener(int idx, bool delay = false);
		FLAME_FOUNDATION_EXPORTS void remove_changed_listener(int idx, bool delay = false);
		FLAME_FOUNDATION_EXPORTS void remove_child_listener(int idx, bool delay = false);

		FLAME_UNIVERSE_EXPORTS SerializableNode* save();

		enum { DATA_SIZE = 0 };

		FLAME_UNIVERSE_EXPORTS static Element* create(UI* ui);
		template<typename T, typename ... Args>
		inline static T* createT(UI* ui, Args ... args)
		{
			auto w = (T*)create(ins);
			w->init(args...);
			return w;
		}
		FLAME_UNIVERSE_EXPORTS static void create_from_typeinfo(UI* ui, VaribleInfo* info, void* p, Element* dst); // use variable to create element, e.g. string->edit, bool->checkbox
		FLAME_UNIVERSE_EXPORTS static Element* create_from_file(UI* ui, SerializableNode* src);
		FLAME_UNIVERSE_EXPORTS static void destroy(Element* w);
	};

#define FLAME_ELEMENT_BEGIN(name, base) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { D_BASE = __COUNTER__ + 1 };\
		enum { B_SIZE = base::DATA_SIZE };
#define FLAME_ELEMENT_DATA(t, n, tf) \
		inline t &n()\
		{\
			return (t&)datas$[__COUNTER__ - D_BASE + B_SIZE].tf();\
		}
#define FLAME_ELEMENT_END \
		enum { DATA_SIZE = __COUNTER__ - D_BASE + B_SIZE };\
	};

	FLAME_ELEMENT_BEGIN(wLayout, Element)
		FLAME_UNIVERSE_EXPORTS void init(LayoutType type = LayoutFree, float item_padding = 0.f);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wCheckbox, Element)
		FLAME_UNIVERSE_EXPORTS void init(void* target = nullptr);
		FLAME_ELEMENT_DATA(int, checked, i1)
		FLAME_ELEMENT_DATA(voidptr, target, p)
		
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wText, Element)
		FLAME_UNIVERSE_EXPORTS void init();
		FLAME_ELEMENT_DATA(Bvec4, text_col, b4)
		FLAME_ELEMENT_DATA(float, sdf_scale, f1)
		
		FLAME_UNIVERSE_EXPORTS void set_size_auto();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wButton, wText)
		FLAME_UNIVERSE_EXPORTS void init(const wchar_t* title);
	
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wToggle, wText)
		FLAME_UNIVERSE_EXPORTS void init();
		FLAME_ELEMENT_DATA(int, toggled, i1)
		
		FLAME_UNIVERSE_EXPORTS void set_toggle(bool v);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wMenuItem, wText)
		FLAME_UNIVERSE_EXPORTS void init(const wchar_t* title);
	
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wMenu, wLayout)
		FLAME_UNIVERSE_EXPORTS void init(const wchar_t* title, bool only_for_context_menu = false);
		FLAME_ELEMENT_DATA(int, sub, i1)
		FLAME_ELEMENT_DATA(int, opened, i1)
		FLAME_ELEMENT_DATA(wTextPtr, w_title, p)
		FLAME_ELEMENT_DATA(wTextPtr, w_rarrow, p)
		FLAME_ELEMENT_DATA(wLayoutPtr, w_items, p)
		
		FLAME_UNIVERSE_EXPORTS void open();
		FLAME_UNIVERSE_EXPORTS void popup(const Vec2& pos);
		FLAME_UNIVERSE_EXPORTS void close();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wMenuBar, wLayout)
		FLAME_UNIVERSE_EXPORTS void init();
	
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wCombo, wMenu)
		FLAME_UNIVERSE_EXPORTS void init(void* enum_info = nullptr, void* target = nullptr);
		FLAME_ELEMENT_DATA(int, sel, i1)
		FLAME_ELEMENT_DATA(voidptr, enum_info, p)
		FLAME_ELEMENT_DATA(voidptr, target, p)
		
		FLAME_UNIVERSE_EXPORTS void set_sel(int idx, bool from_inner = false);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wEdit, wText)
		enum Type
		{
			TypeNull,
			TypeString,
			TypeStringW,
			TypeInt,
			TypeUint,
			TypeFloat,
			TypeUchar
		};
		FLAME_UNIVERSE_EXPORTS void init(Type type = TypeNull, void* target = nullptr);
		FLAME_ELEMENT_DATA(int, cursor, i1)
		FLAME_ELEMENT_DATA(int, type, i1)
		FLAME_ELEMENT_DATA(voidptr, target, p)
		
		FLAME_UNIVERSE_EXPORTS void set_size_by_width(float width);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wImage, Element)
		FLAME_UNIVERSE_EXPORTS void init();
		FLAME_ELEMENT_DATA(int, id, i1)
		FLAME_ELEMENT_DATA(Vec2, uv0, f2)
		FLAME_ELEMENT_DATA(Vec2, uv1, f2)
		FLAME_ELEMENT_DATA(int, stretch, i1)
		FLAME_ELEMENT_DATA(Vec4, border, f4) // L R T B
		
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wSizeDrag, Element)
		FLAME_UNIVERSE_EXPORTS void init(Element* target);
		FLAME_ELEMENT_DATA(Vec2, min_size, f2)
		
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wScrollbar, wLayout)
		FLAME_UNIVERSE_EXPORTS void init(Element* target);
		FLAME_ELEMENT_DATA(wButtonPtr, w_btn, p)
		FLAME_ELEMENT_DATA(ElementPtr, w_target, p)
		
		FLAME_UNIVERSE_EXPORTS void scroll(int v);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wListItem, wLayout)
		FLAME_UNIVERSE_EXPORTS void init(const wchar_t* title);
		FLAME_ELEMENT_DATA(wTextPtr, w_title, p)
		
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wList, wLayout)
		FLAME_UNIVERSE_EXPORTS void init();
		FLAME_ELEMENT_DATA(wListItemPtr, w_sel, p)
		FLAME_ELEMENT_DATA(wScrollbarPtr, w_scrollbar, p)
		
	FLAME_ELEMENT_END

	struct wTree;
	FLAME_ELEMENT_BEGIN(wTreeNode, wLayout)
		FLAME_UNIVERSE_EXPORTS void init(const wchar_t* title, wTree* tree = nullptr);
		FLAME_ELEMENT_DATA(wTextPtr, w_title, p)
		FLAME_ELEMENT_DATA(wLayoutPtr, w_items, p)
		FLAME_ELEMENT_DATA(wTextPtr, w_larrow, p)
		
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wTree, wLayout)
		FLAME_UNIVERSE_EXPORTS void init();
		FLAME_ELEMENT_DATA(wTreeNodePtr, w_sel, p)
		
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wDialog, wLayout)
		FLAME_UNIVERSE_EXPORTS void init(bool resize = false, bool modual = false);
		FLAME_ELEMENT_DATA(wScrollbarPtr, w_scrollbar, p)
		FLAME_ELEMENT_DATA(wSizeDragPtr, w_sizedrag, p)
		
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN(wMessageDialog, wDialog)
		FLAME_UNIVERSE_EXPORTS void init(const wchar_t* text);
		FLAME_ELEMENT_DATA(wTextPtr, w_text, p)
		FLAME_ELEMENT_DATA(wButtonPtr, w_ok, p)
		
	FLAME_ELEMENT_END

	//struct wYesNoDialog : wDialog
	//{
	//	FLAME_UNIVERSE_EXPORTS void init(const wchar_t *text, const wchar_t *prompt, cconst std::function<void(bool)> &callback);

	//	FLAME_UNIVERSE_EXPORTS wTextPtr &w_text();
	//	FLAME_UNIVERSE_EXPORTS wLayoutPtr &w_buttons();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_yes();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_no();
	//};

	//struct wInputDialog : wDialog
	//{
	//	FLAME_UNIVERSE_EXPORTS void init(const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback);

	//	FLAME_UNIVERSE_EXPORTS wEditPtr &w_input();
	//	FLAME_UNIVERSE_EXPORTS wLayoutPtr &w_buttons();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_ok();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_cancel();
	//};

	//struct wFileDialog : wDialog
	//{
	//	FLAME_UNIVERSE_EXPORTS void init(const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts = nullptr);

	//	FLAME_UNIVERSE_EXPORTS wMenuBarPtr &w_pathstems();
	//	FLAME_UNIVERSE_EXPORTS wListPtr &w_list();
	//	FLAME_UNIVERSE_EXPORTS wEditPtr &w_input();
	//	FLAME_UNIVERSE_EXPORTS wComboPtr &w_ext();
	//	FLAME_UNIVERSE_EXPORTS wLayoutPtr &w_buttons();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_ok();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_cancel();

	//	FLAME_UNIVERSE_EXPORTS const wchar_t *curr_path();
	//	FLAME_UNIVERSE_EXPORTS int curr_path_len();
	//	FLAME_UNIVERSE_EXPORTS void set_curr_path(const wchar_t *path);
	//	FLAME_UNIVERSE_EXPORTS const wchar_t *curr_exts();
	//	FLAME_UNIVERSE_EXPORTS int curr_exts_len();
	//	FLAME_UNIVERSE_EXPORTS void set_curr_exts(const wchar_t *exts);

	//	FLAME_UNIVERSE_EXPORTS void set_path(const wchar_t *path);
	//};
}
