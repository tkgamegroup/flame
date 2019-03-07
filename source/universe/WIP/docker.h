const char* get_dock_dir_name(DockDirection dir);

float get_layout_padding(bool horizontal);

struct Layout
{
	Layout* parent;
	int idx;

	LayoutType type;

	float width;
	float height;
	float size_radio;
	ImGui::Splitter splitter;

	std::unique_ptr<Layout> children[2];
	std::list<Window*> windows[2];
	Window* curr_tab[2];

	Layout();
	bool is_empty(int idx) const;
	bool is_empty() const;
	void set_size();
	void set_layout(int idx, Layout* l);
	void set_layout(int idx, std::unique_ptr<Layout>&& l);
	void add_window(int idx, Window* w);
	void remove_window(int idx, Window* w);
	void clear_window(int idx);
	void show_window(int idx);
	void show();
};

extern Layout* main_layout;

void cleanup_layout();
void resize_layout();
void dock(Window* src, Window* dst = nullptr, DockDirection dir = DockCenter);
void undock(Window* w);
void set_dragging_window(Window* w);
void load_layout();
void save_layout();
void reset_dragging();
void show_layout();