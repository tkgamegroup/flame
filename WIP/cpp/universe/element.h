namespace flame
{
	struct Element
	{
		Array<Animation> animations$;

		FLAME_PACKAGE_BEGIN_3(FoucusListenerParm, ElementPtr, thiz, p, FocusType, type, i1, int, is_keyfocus, i1)
		FLAME_PACKAGE_END_3

		FLAME_PACKAGE_BEGIN_2(DropListenerParm, ElementPtr, thiz, p, ElementPtr, src, p)
		FLAME_PACKAGE_END_2

		FLAME_PACKAGE_BEGIN_1(ChangedListenerParm, ElementPtr, thiz, p)
		FLAME_PACKAGE_END_1

		FLAME_UNIVERSE_EXPORTS static void create_from_typeinfo(UI* ui, int font_atlas_index, VariableInfo* info, void* p, Element* dst); // use variable to create element, e.g. string->edit, bool->checkbox
	};

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

	FLAME_ELEMENT_BEGIN_0(wDocker, Element)
		enum Dir
		{
			DirCenter = 1 << 0,
			DirLeft = 1 << 1,
			DirRight = 1 << 2,
			DirTop = 1 << 3,
			DirBottom = 1 << 4,
			DirAll = DirCenter | DirLeft | DirRight | DirTop | DirBottom
		};

		enum Type
		{
			TypeCenter,
			TypeHorizontal,
			TypeVertical
		};

		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END
}
