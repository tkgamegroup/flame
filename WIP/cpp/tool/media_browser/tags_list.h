struct TagsListLayout : flame::UI::Layout
{
	flame::UI::Layout *top;
	flame::UI::Button *clear;
	flame::UI::Edit *filter;
	flame::UI::List *list;

	TagsListLayout(flame::UI::Instance *ui);
};

struct TagsListItem : flame::UI::ListItem
{
	flame::UI::Checkbox *w_c;
	flame::UI::Layout *w_right;
	flame::UI::Text *w_t;
	flame::UI::Button *w_edt;
	flame::UI::Button *w_del;
	Tag *t;

	TagsListItem(flame::UI::Instance *ui, Tag *_t);
};

void create_tags_list();
void refresh_tags_list();
void update_tag(TagsListItem *item);
void update_all_tags();
