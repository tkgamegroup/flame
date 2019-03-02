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
#include <flame/universe/style.h>
#include <flame/universe/animation.h>

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
		enum Flag
		{
			FlagNull,
			FlagJustCreated,
			FlagJustCreatedNeedModual,
			FlagNeedToRemoveFromParent,
			FlagNeedToTakeFromParent
		};

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
		bool showed; // vaild after processing
		State state; // vaild after processing

		UI* ui;
		Element* parent;
		int layer;
		Flag flag;
		bool need_arrange;

		Array<Element*> children_1$;
		Array<Element*> children_2$;

		bool draw_default$;
		FLAME_PACKAGE_BEGIN_4(ExtraDrawParm, ElementPtr, thiz, p, graphics::CanvasPtr, canvas, p, Vec2, off, f2, float, scl, f1)
		FLAME_PACKAGE_END_4
		Array<Function<ExtraDrawParm>> extra_draws$;

		int closet_id$;
		int style_level;
		Array<Style> styles$;

		Array<Animation> animations$;

		FLAME_PACKAGE_BEGIN_3(FoucusListenerParm, ElementPtr, thiz, p, FocusType, type, i1, int, is_keyfocus, i1)
		FLAME_PACKAGE_END_3

		FLAME_PACKAGE_BEGIN_3(KeyListenerParm, ElementPtr, thiz, p, KeyState, action, i1, int, value, i1)
		/*
			- when key down/up, action is KeyStateDown or KeyStateUp, value is Key
			- when char, action is KeyStateNull, value is ch
		*/
			inline bool is_down() { return action() == KeyStateDown; }
			inline bool is_up()   { return action() == KeyStateUp;   }
			inline bool is_char() { return action() == KeyStateNull; }
		FLAME_PACKAGE_END_3

		FLAME_PACKAGE_BEGIN_4(MouseListenerParm, ElementPtr, thiz, p, KeyState, action, i1, MouseKey, key, i1, Vec2, value, f2)
		/*
			- when enter/leave, action is KeyStateDown or KeyStateUp, key is Mouse_Null
			- when down/up, action is KeyStateDown or KeyStateUp, key is MouseKey, value is pos
			- when move, action is KeyStateNull, key is Mouse_Null, value is disp
			- when scroll, action is KeyStateNull, key is Mouse_Middle, value.x is scroll value
			- when clicked, action is KeyStateDown | KeyStateUp | (KeyStateDouble ? for double clicked), key is Mouse_Null
		*/
			inline bool is_enter()  { return action() == KeyStateDown && key() == Mouse_Null;   }
			inline bool is_leave()  { return action() == KeyStateUp   && key() == Mouse_Null;   }
			inline bool is_down()   { return action() == KeyStateDown && key() != Mouse_Null;   }
			inline bool is_up()     { return action() == KeyStateUp   && key() != Mouse_Null;   }
			inline bool is_move()   { return action() == KeyStateNull && key() == Mouse_Null;   }
			inline bool is_scroll() { return action() == KeyStateNull && key() == Mouse_Middle; }
			inline bool is_clicked(){ return action() == (KeyStateDown | KeyStateUp) && key() == Mouse_Null; }
			inline bool is_double_clicked() { return action() == (KeyStateDown | KeyStateUp | KeyStateDouble) && key() == Mouse_Null; }
		FLAME_PACKAGE_END_4

		FLAME_PACKAGE_BEGIN_2(DropListenerParm, ElementPtr, thiz, p, ElementPtr, src, p)
		FLAME_PACKAGE_END_2

		FLAME_PACKAGE_BEGIN_1(ChangedListenerParm, ElementPtr, thiz, p)
		FLAME_PACKAGE_END_1

		enum ChildOp
		{
			ChildAdd,
			ChildRemove
		};
		FLAME_PACKAGE_BEGIN_3(ChildListenerParm, ElementPtr, thiz, p, ChildOp, op, i1, ElementPtr, src, p)
		FLAME_PACKAGE_END_3

		Array<Function<FoucusListenerParm>> focus_listeners$;
		Array<Function<KeyListenerParm>> key_listeners$;
		Array<Function<MouseListenerParm>> mouse_listeners$;
		Array<Function<DropListenerParm>> drop_listeners$;
		Array<Function<ChangedListenerParm>> changed_listeners$;
		Array<Function<ChildListenerParm>> child_listeners$;

		Array<CommonData> datas$;
		StringW text$;

		FLAME_UNIVERSE_EXPORTS Element(UI* ui);
		FLAME_UNIVERSE_EXPORTS ~Element();

		FLAME_UNIVERSE_EXPORTS void set_width(float x, Element* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height(float y, Element* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2& v, Element* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS void set_visibility(bool v);

		FLAME_UNIVERSE_EXPORTS void add_child(Element* w, int layer = 0, int pos = -1, bool modual = false);
		FLAME_UNIVERSE_EXPORTS void remove_child(int layer, int idx, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_child(int layer, int idx, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void clear_children(int layer, int begin, int end, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_children(int layer, int begin, int end, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void remove_from_parent(bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_from_parent(bool delay = false);
		FLAME_UNIVERSE_EXPORTS int find_child(int layer, Element* w);
		FLAME_UNIVERSE_EXPORTS void set_to_foreground();

		FLAME_UNIVERSE_EXPORTS void do_arrange(); // only ui can call this

		FLAME_UNIVERSE_EXPORTS float get_content_size() const;

		FLAME_UNIVERSE_EXPORTS void remove_animations();

		FLAME_UNIVERSE_EXPORTS void on_draw(graphics::Canvas* c, const Vec2& off, float scl);
		FLAME_UNIVERSE_EXPORTS void on_focus(FocusType type, int is_keyfocus);
		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, int value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2& value);
		FLAME_UNIVERSE_EXPORTS void on_drop(Element* src);
		FLAME_UNIVERSE_EXPORTS void on_changed();

		FLAME_UNIVERSE_EXPORTS SerializableNode* save();

		enum { DATA_SIZE = 0 };

		FLAME_UNIVERSE_EXPORTS static Element* create(UI* ui);
		template<typename T, typename ... Args>
		inline static T* createT(UI* ui, Args ... args)
		{
			auto w = (T*)create(ui);
			w->init(args...);
			return w;
		}
		FLAME_UNIVERSE_EXPORTS static void create_from_typeinfo(UI* ui, int font_atlas_index, VaribleInfo* info, void* p, Element* dst); // use variable to create element, e.g. string->edit, bool->checkbox
		FLAME_UNIVERSE_EXPORTS static Element* create_from_file(UI* ui, SerializableNode* src);
		FLAME_UNIVERSE_EXPORTS static void destroy(Element* w);
	};

#define FLAME_ELEMENT_BEGIN_0(name, base) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 0 + B_SIZE };\
		inline void init_data_types()\
		{\
			class$ = #name;\
		}
#define FLAME_ELEMENT_END \
	};

	// t is type, n is name, tf is type format
	// remember to call 'init_data_types' in 'init'

#define FLAME_ELEMENT_BEGIN_1(name, base, t1, n1, tf1) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 1 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(1 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
		}

#define FLAME_ELEMENT_BEGIN_2(name, base, t1, n1, tf1, t2, n2, tf2) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 2 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline t2 &n2()\
		{\
			return (t2&)datas$[1 + B_SIZE].tf2();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(2 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
			str_to_typefmt(datas$[1 + B_SIZE].fmt, #tf2);\
		}

#define FLAME_ELEMENT_BEGIN_3(name, base, t1, n1, tf1, t2, n2, tf2, t3, n3, tf3) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 3 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline t2 &n2()\
		{\
			return (t2&)datas$[1 + B_SIZE].tf2();\
		}\
		inline t3 &n3()\
		{\
			return (t3&)datas$[2 + B_SIZE].tf3();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(3 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
			str_to_typefmt(datas$[1 + B_SIZE].fmt, #tf2);\
			str_to_typefmt(datas$[2 + B_SIZE].fmt, #tf3);\
		}

#define FLAME_ELEMENT_BEGIN_4(name, base, t1, n1, tf1, t2, n2, tf2, t3, n3, tf3, t4, n4, tf4) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 4 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline t2 &n2()\
		{\
			return (t2&)datas$[1 + B_SIZE].tf2();\
		}\
		inline t3 &n3()\
		{\
			return (t3&)datas$[2 + B_SIZE].tf3();\
		}\
		inline t4 &n4()\
		{\
			return (t4&)datas$[3 + B_SIZE].tf4();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(4 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
			str_to_typefmt(datas$[1 + B_SIZE].fmt, #tf2);\
			str_to_typefmt(datas$[2 + B_SIZE].fmt, #tf3);\
			str_to_typefmt(datas$[3 + B_SIZE].fmt, #tf4);\
		}

#define FLAME_ELEMENT_BEGIN_5(name, base, t1, n1, tf1, t2, n2, tf2, t3, n3, tf3, t4, n4, tf4, t5, n5, tf5) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 5 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline t2 &n2()\
		{\
			return (t2&)datas$[1 + B_SIZE].tf2();\
		}\
		inline t3 &n3()\
		{\
			return (t3&)datas$[2 + B_SIZE].tf3();\
		}\
		inline t4 &n4()\
		{\
			return (t4&)datas$[3 + B_SIZE].tf4();\
		}\
		inline t5 &n5()\
		{\
			return (t5&)datas$[4 + B_SIZE].tf5();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(5 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
			str_to_typefmt(datas$[1 + B_SIZE].fmt, #tf2);\
			str_to_typefmt(datas$[2 + B_SIZE].fmt, #tf3);\
			str_to_typefmt(datas$[3 + B_SIZE].fmt, #tf4);\
			str_to_typefmt(datas$[4 + B_SIZE].fmt, #tf5);\
		}

#define FLAME_ELEMENT_BEGIN_6(name, base, t1, n1, tf1, t2, n2, tf2, t3, n3, tf3, t4, n4, tf4, t5, n5, tf5, t6, n6, tf6) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 6 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline t2 &n2()\
		{\
			return (t2&)datas$[1 + B_SIZE].tf2();\
		}\
		inline t3 &n3()\
		{\
			return (t3&)datas$[2 + B_SIZE].tf3();\
		}\
		inline t4 &n4()\
		{\
			return (t4&)datas$[3 + B_SIZE].tf4();\
		}\
		inline t5 &n5()\
		{\
			return (t5&)datas$[4 + B_SIZE].tf5();\
		}\
		inline t6 &n6()\
		{\
			return (t6&)datas$[5 + B_SIZE].tf6();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(6 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
			str_to_typefmt(datas$[1 + B_SIZE].fmt, #tf2);\
			str_to_typefmt(datas$[2 + B_SIZE].fmt, #tf3);\
			str_to_typefmt(datas$[3 + B_SIZE].fmt, #tf4);\
			str_to_typefmt(datas$[4 + B_SIZE].fmt, #tf5);\
			str_to_typefmt(datas$[5 + B_SIZE].fmt, #tf6);\
		}

#define FLAME_ELEMENT_BEGIN_7(name, base, t1, n1, tf1, t2, n2, tf2, t3, n3, tf3, t4, n4, tf4, t5, n5, tf5, t6, n6, tf6, t7, n7, tf7) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 7 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline t2 &n2()\
		{\
			return (t2&)datas$[1 + B_SIZE].tf2();\
		}\
		inline t3 &n3()\
		{\
			return (t3&)datas$[2 + B_SIZE].tf3();\
		}\
		inline t4 &n4()\
		{\
			return (t4&)datas$[3 + B_SIZE].tf4();\
		}\
		inline t5 &n5()\
		{\
			return (t5&)datas$[4 + B_SIZE].tf5();\
		}\
		inline t6 &n6()\
		{\
			return (t6&)datas$[5 + B_SIZE].tf6();\
		}\
		inline t7 &n7()\
		{\
			return (t7&)datas$[6 + B_SIZE].tf7();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(7 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
			str_to_typefmt(datas$[1 + B_SIZE].fmt, #tf2);\
			str_to_typefmt(datas$[2 + B_SIZE].fmt, #tf3);\
			str_to_typefmt(datas$[3 + B_SIZE].fmt, #tf4);\
			str_to_typefmt(datas$[4 + B_SIZE].fmt, #tf5);\
			str_to_typefmt(datas$[5 + B_SIZE].fmt, #tf6);\
			str_to_typefmt(datas$[6 + B_SIZE].fmt, #tf7);\
		}

#define FLAME_ELEMENT_BEGIN_8(name, base, t1, n1, tf1, t2, n2, tf2, t3, n3, tf3, t4, n4, tf4, t5, n5, tf5, t6, n6, tf6, t7, n7, tf7, t8, n8, tf8) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { B_SIZE = base::DATA_SIZE };\
		enum { DATA_SIZE = 8 + B_SIZE };\
		inline t1 &n1()\
		{\
			return (t1&)datas$[0 + B_SIZE].tf1();\
		}\
		inline t2 &n2()\
		{\
			return (t2&)datas$[1 + B_SIZE].tf2();\
		}\
		inline t3 &n3()\
		{\
			return (t3&)datas$[2 + B_SIZE].tf3();\
		}\
		inline t4 &n4()\
		{\
			return (t4&)datas$[3 + B_SIZE].tf4();\
		}\
		inline t5 &n5()\
		{\
			return (t5&)datas$[4 + B_SIZE].tf5();\
		}\
		inline t6 &n6()\
		{\
			return (t6&)datas$[5 + B_SIZE].tf6();\
		}\
		inline t7 &n7()\
		{\
			return (t7&)datas$[6 + B_SIZE].tf7();\
		}\
		inline t8 &n8()\
		{\
			return (t8&)datas$[7 + B_SIZE].tf8();\
		}\
		inline void init_data_types()\
		{\
			class$ = #name;\
			datas$.resize(8 + B_SIZE);\
			str_to_typefmt(datas$[0 + B_SIZE].fmt, #tf1);\
			str_to_typefmt(datas$[1 + B_SIZE].fmt, #tf2);\
			str_to_typefmt(datas$[2 + B_SIZE].fmt, #tf3);\
			str_to_typefmt(datas$[3 + B_SIZE].fmt, #tf4);\
			str_to_typefmt(datas$[4 + B_SIZE].fmt, #tf5);\
			str_to_typefmt(datas$[5 + B_SIZE].fmt, #tf6);\
			str_to_typefmt(datas$[6 + B_SIZE].fmt, #tf7);\
			str_to_typefmt(datas$[7 + B_SIZE].fmt, #tf8);\
		}

	FLAME_ELEMENT_BEGIN_0(wLayout, Element)
		FLAME_UNIVERSE_EXPORTS void init(LayoutType type = LayoutFree, float item_padding = 0.f);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wCheckbox, Element, int, checked, i1, voidptr, target, p)
		FLAME_UNIVERSE_EXPORTS void init(void* target = nullptr);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_3(wText, Element, int, font_atlas_index, i1, Bvec4, text_col, b4, float, sdf_scale, f1)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index);
		FLAME_UNIVERSE_EXPORTS void set_size_auto();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_0(wButton, wText)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_1(wToggle, wText, int, toggled, i1)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index);
		FLAME_UNIVERSE_EXPORTS void set_toggle(bool v);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_0(wMenuItem, wText)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_5(wMenu, wLayout, bool, sub, i1, bool, opened, i1, wTextPtr, w_title, p, wTextPtr, w_rarrow, p, wLayoutPtr, w_items, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title, bool only_for_context_menu = false);
		FLAME_UNIVERSE_EXPORTS void open();
		FLAME_UNIVERSE_EXPORTS void popup(const Vec2& pos);
		FLAME_UNIVERSE_EXPORTS void close();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_0(wMenuBar, wLayout)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_3(wCombo, wMenu, int, sel, i1, voidptr, enum_info, p, voidptr, target, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, void* enum_info = nullptr, void* target = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_sel(int idx, bool from_inner = false);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_3(wEdit, wText, int, cursor, i1, int, type, i1, voidptr, target, p)
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
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, Type type = TypeNull, void* target = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size_by_width(float width);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_5(wImage, Element, int, id, i1, Vec2, uv0, f2, Vec2, uv1, f2, int, stretch, i1, Vec4, border/* L R T B */, f4)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wSizeDrag, Element, ElementPtr, w_target, p, Vec2, min_size, f2)
		FLAME_UNIVERSE_EXPORTS void init(Element* target);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wScrollbar, wLayout, wButtonPtr, w_btn, p, ElementPtr, w_target, p)
		FLAME_UNIVERSE_EXPORTS void init(Element* target);
		FLAME_UNIVERSE_EXPORTS void scroll(int v);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_1(wListItem, wLayout, wTextPtr, w_title, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wList, wLayout, wListItemPtr, w_sel, p, wScrollbarPtr, w_scrollbar, p)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	struct wTree;
	FLAME_ELEMENT_BEGIN_3(wTreeNode, wLayout, wTextPtr, w_title, p, wLayoutPtr, w_items, p, wTextPtr, w_larrow, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title, wTree* tree = nullptr);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_1(wTree, wLayout, wTreeNodePtr, w_sel, p)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wDialog, wLayout, wScrollbarPtr, w_scrollbar, p, wSizeDragPtr, w_sizedrag, p)
		FLAME_UNIVERSE_EXPORTS void init(bool resize = false, bool modual = false);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wMessageDialog, wDialog, wTextPtr, w_text, p, wButtonPtr, w_ok, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* text);
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
